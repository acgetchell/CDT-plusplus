/*******************************************************************************
Causal Dynamical Triangulations in C++ using CGAL
Copyright © 2022 Adam Getchell
******************************************************************************/

/// @file cdt-viewer.cpp
/// @brief Render versioned 3D triangulation fixtures with CGAL and Qt
/// @author Adam Getchell

#include <CGAL/draw_triangulation_3.h>
#include <CGAL/Graphics_scene.h>
#include <CGAL/Graphics_scene_options.h>
#include <CGAL/IO/Color.h>
#include <CGAL/Qt/Basic_viewer.h>
#include <CGAL/Qt/camera.h>
#include <CGAL/Qt/init_ogl_context.h>
#include <CGAL/Qt/qglviewer.h>
#include <CGAL/version.h>
#include <fmt/ostream.h>
#include <fmt/printf.h>

#include <algorithm>
#include <array>
#include <bit>
#include <boost/program_options.hpp>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <limits>
#include <numbers>
#include <QApplication>
#include <QColor>
#include <QCryptographicHash>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QString>
#include <QtGlobal>
#include <QTimer>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "Settings.hpp"
#include "Triangulation_traits.hpp"
#include "Utilities.hpp"
#include "Version.hpp"

namespace po = boost::program_options;

namespace
{
  using Delaunay      = cdt::detail::TriangulationTraits<3>::Delaunay;
  using Triangulation = Delaunay::Tr_Base;
  using Scene_options =
      CGAL::Graphics_scene_options<Triangulation, Triangulation::Vertex_handle,
                                   Triangulation::Finite_edges_iterator,
                                   Triangulation::Finite_facets_iterator>;

  constexpr std::string_view USAGE =
      R"(Causal Dynamical Triangulations in C++ using CGAL.

Copyright (c) 2022 Adam Getchell

Load a versioned triangulation fixture and render it with the CGAL Qt viewer.
Without --output, the viewer remains interactive. With --output it saves the
manifest-declared frame and exits without waiting for user input.

Usage:
  cdt-viewer --manifest MANIFEST [--fixture FIXTURE] [--output IMAGE]

Options)";

  struct Topology_counts
  {
    std::size_t        vertices{};
    std::size_t        edges{};
    std::size_t        faces{};
    std::size_t        simplices{};
    cdt::Int_precision minimum_timeslice{};
    cdt::Int_precision maximum_timeslice{};
  };

  struct Camera_config
  {
    std::string           projection;
    std::array<double, 3> position{};
    std::array<double, 3> target{};
    std::array<double, 3> up{};
    double                vertical_field_of_view_radians{};
  };

  struct Render_config
  {
    int                          width{};
    int                          height{};
    bool                         transparent_background{};
    double                       oversampling{};
    bool                         expand_frustum{};
    QColor                       background;
    bool                         draw_vertices{};
    bool                         draw_edges{};
    bool                         draw_faces{};
    std::string                  vertex_scope;
    std::string                  edge_scope;
    float                        point_size{};
    float                        line_width{};
    int                          edge_color_difference_threshold{};
    bool                         flat_shading{};
    QColor                       point_color;
    QColor                       edge_color;
    std::vector<CGAL::IO::Color> face_palette;
    Camera_config                camera;
    std::size_t                  minimum_foreground_pixels{};
  };

  struct Viewer_manifest
  {
    std::filesystem::path fixture;
    std::string           fixture_sha256;
    Topology_counts       expected;
    std::string           cgal_version;
    std::string           qt_version;
    Render_config         render;
  };

  class Artifact_viewer final : public CGAL::Qt::Basic_viewer
  {
   public:
    using CGAL::Qt::Basic_viewer::Basic_viewer;

    void after_initialization(std::function<void()> callback)
    { callback_ = std::move(callback); }

   protected:
    void init() override
    {
      CGAL::Qt::Basic_viewer::init();
      glDisable(GL_DITHER);
      glDisable(GL_LINE_SMOOTH);
      glDisable(GL_MULTISAMPLE);
      if (callback_)
      {
        QTimer::singleShot(0, this, [this]() { callback_(); });
      }
    }

   private:
    std::function<void()> callback_;
  };

  [[nodiscard]] auto require_object(QJsonObject const& parent,
                                    QString const&     key) -> QJsonObject
  {
    auto const value = parent.value(key);
    if (!value.isObject())
    {
      throw std::invalid_argument(fmt::format(
          "Render manifest field '{}' must be an object.", key.toStdString()));
    }
    return value.toObject();
  }

  [[nodiscard]] auto require_array(QJsonObject const& parent,
                                   QString const&     key) -> QJsonArray
  {
    auto const value = parent.value(key);
    if (!value.isArray())
    {
      throw std::invalid_argument(fmt::format(
          "Render manifest field '{}' must be an array.", key.toStdString()));
    }
    return value.toArray();
  }

  [[nodiscard]] auto require_string(QJsonObject const& parent,
                                    QString const&     key) -> std::string
  {
    auto const value = parent.value(key);
    if (!value.isString() || value.toString().isEmpty())
    {
      throw std::invalid_argument(
          fmt::format("Render manifest field '{}' must be a nonempty string.",
                      key.toStdString()));
    }
    return value.toString().toStdString();
  }

  [[nodiscard]] auto require_bool(QJsonObject const& parent, QString const& key)
      -> bool
  {
    auto const value = parent.value(key);
    if (!value.isBool())
    {
      throw std::invalid_argument(fmt::format(
          "Render manifest field '{}' must be Boolean.", key.toStdString()));
    }
    return value.toBool();
  }

  [[nodiscard]] auto require_number(QJsonObject const& parent,
                                    QString const&     key) -> double
  {
    auto const value = parent.value(key);
    if (!value.isDouble() || !std::isfinite(value.toDouble()))
    {
      throw std::invalid_argument(
          fmt::format("Render manifest field '{}' must be a finite number.",
                      key.toStdString()));
    }
    return value.toDouble();
  }

  constexpr long double MAX_EXACT_JSON_INTEGER{9'007'199'254'740'991.0L};

  template <std::integral Integer>
  [[nodiscard]] auto require_integer(QJsonObject const& parent,
                                     QString const&     key) -> Integer
  {
    auto const number  = require_number(parent, key);
    auto const widened = static_cast<long double>(number);
    if (std::trunc(number) != number ||
        widened <
            static_cast<long double>(std::numeric_limits<Integer>::min()) ||
        widened >
            static_cast<long double>(std::numeric_limits<Integer>::max()) ||
        widened < -MAX_EXACT_JSON_INTEGER || widened > MAX_EXACT_JSON_INTEGER)
    {
      throw std::invalid_argument(
          fmt::format("Render manifest field '{}' must fit the requested "
                      "integer type.",
                      key.toStdString()));
    }
    return static_cast<Integer>(number);
  }

  [[nodiscard]] auto require_vector(QJsonObject const& parent,
                                    QString const& key) -> std::array<double, 3>
  {
    auto const values = require_array(parent, key);
    if (values.size() != 3)
    {
      throw std::invalid_argument(
          fmt::format("Render manifest field '{}' must have three entries.",
                      key.toStdString()));
    }
    std::array<double, 3> result{};
    for (qsizetype index = 0; index < values.size(); ++index)
    {
      auto const value = values.at(index);
      if (!value.isDouble() || !std::isfinite(value.toDouble()))
      {
        throw std::invalid_argument(
            fmt::format("Render manifest field '{}' contains a non-number.",
                        key.toStdString()));
      }
      result.at(static_cast<std::size_t>(index)) = value.toDouble();
    }
    return result;
  }

  [[nodiscard]] auto require_color(QJsonObject const& parent,
                                   QString const&     key) -> QColor
  {
    auto const values = require_array(parent, key);
    if (values.size() != 4)
    {
      throw std::invalid_argument(fmt::format(
          "Render manifest field '{}' must be RGBA.", key.toStdString()));
    }
    std::array<int, 4> channels{};
    for (qsizetype index = 0; index < values.size(); ++index)
    {
      auto const value = values.at(index);
      if (!value.isDouble() ||
          std::trunc(value.toDouble()) != value.toDouble() ||
          value.toDouble() < 0.0 || value.toDouble() > 255.0)
      {
        throw std::invalid_argument(
            fmt::format("Render manifest color '{}' must use integer channels "
                        "from 0 through 255.",
                        key.toStdString()));
      }
      channels.at(static_cast<std::size_t>(index)) =
          static_cast<int>(value.toDouble());
    }
    return {channels[0], channels[1], channels[2], channels[3]};
  }

  [[nodiscard]] auto to_cgal_color(QColor const& color) -> CGAL::IO::Color
  {
    return {static_cast<unsigned char>(color.red()),
            static_cast<unsigned char>(color.green()),
            static_cast<unsigned char>(color.blue()),
            static_cast<unsigned char>(color.alpha())};
  }

  [[nodiscard]] auto parse_manifest(std::filesystem::path const& path)
      -> Viewer_manifest
  {
    QFile file(QString::fromStdString(path.string()));
    if (!file.open(QIODevice::ReadOnly))
    {
      throw std::filesystem::filesystem_error(
          "Could not open render manifest", path,
          std::make_error_code(std::errc::no_such_file_or_directory));
    }

    QJsonParseError error;
    auto const      document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject())
    {
      throw std::invalid_argument(
          fmt::format("Could not parse render manifest {}: {}", path.string(),
                      error.errorString().toStdString()));
    }
    auto const root = document.object();
    if (require_integer<int>(root, "schema_version") != 1)
    {
      throw std::invalid_argument("Unsupported render manifest schema.");
    }

    auto const fixture  = require_object(root, "fixture");
    auto const topology = require_object(fixture, "expected_topology");
    auto const renderer = require_object(root, "renderer");
    auto const render   = require_object(root, "render");
    auto const geometry = require_object(render, "geometry");
    auto const style    = require_object(render, "style");
    auto const camera   = require_object(render, "camera");
    auto const checks   = require_object(root, "comparison");

    auto fixture_path   = path.parent_path() / require_string(fixture, "path");
    fixture_path        = std::filesystem::weakly_canonical(fixture_path);

    std::vector<CGAL::IO::Color> palette;
    for (auto const entry : require_array(style, "face_palette"))
    {
      if (!entry.isArray())
      {
        throw std::invalid_argument(
            "Every face palette entry must be an RGB array.");
      }
      auto const channels = entry.toArray();
      if (channels.size() != 3)
      {
        throw std::invalid_argument(
            "Every face palette entry must have three channels.");
      }
      std::array<unsigned char, 3> color{};
      for (qsizetype index = 0; index < channels.size(); ++index)
      {
        auto const channel = channels.at(index);
        if (!channel.isDouble() ||
            std::trunc(channel.toDouble()) != channel.toDouble() ||
            channel.toDouble() < 0.0 || channel.toDouble() > 255.0)
        {
          throw std::invalid_argument(
              "Face palette channels must be integers from 0 through 255.");
        }
        color.at(static_cast<std::size_t>(index)) =
            static_cast<unsigned char>(channel.toDouble());
      }
      palette.emplace_back(color[0], color[1], color[2]);
    }
    if (palette.empty())
    {
      throw std::invalid_argument("The face palette must not be empty.");
    }

    auto const width        = require_integer<int>(render, "width");
    auto const height       = require_integer<int>(render, "height");
    auto const point_size   = require_number(style, "point_size");
    auto const line_width   = require_number(style, "line_width");
    auto const oversampling = require_number(render, "oversampling");
    if (width <= 0 || height <= 0 || point_size <= 0.0 || line_width <= 0.0 ||
        oversampling < 1.0 ||
        point_size > static_cast<double>(std::numeric_limits<float>::max()) ||
        line_width > static_cast<double>(std::numeric_limits<float>::max()))
    {
      throw std::invalid_argument(
          "Render dimensions and point/line sizes must be positive and "
          "representable.");
    }
    if (require_string(render, "output_format") != "png")
    {
      throw std::invalid_argument("The v1 viewer output format must be PNG.");
    }

    Viewer_manifest result{
        .fixture        = std::move(fixture_path),
        .fixture_sha256 = require_string(fixture, "sha256"),
        .expected = {.vertices =
                         require_integer<std::size_t>(topology, "vertices"),
                     .edges = require_integer<std::size_t>(topology, "edges"),
                     .faces = require_integer<std::size_t>(topology, "faces"),
                     .simplices =
                         require_integer<std::size_t>(topology, "simplices"),
                     .minimum_timeslice = require_integer<cdt::Int_precision>(
                         topology, "minimum_timeslice"),
                     .maximum_timeslice = require_integer<cdt::Int_precision>(
                         topology, "maximum_timeslice")},
        .cgal_version = require_string(renderer, "cgal_version"),
        .qt_version   = require_string(renderer, "qt_version"),
        .render       = {
                     .width  = width,
                     .height = height,
                     .transparent_background =
                require_bool(render, "transparent_background"),
                     .oversampling   = oversampling,
                     .expand_frustum = require_bool(render, "expand_frustum"),
                     .background     = require_color(style, "background_rgba"),
                     .draw_vertices  = require_bool(geometry, "vertices"),
                     .draw_edges     = require_bool(geometry, "edges"),
                     .draw_faces     = require_bool(geometry, "faces"),
                     .vertex_scope   = require_string(geometry, "vertex_scope"),
                     .edge_scope     = require_string(geometry, "edge_scope"),
                     .point_size     = static_cast<float>(point_size),
                     .line_width     = static_cast<float>(line_width),
                     .edge_color_difference_threshold =
                require_integer<int>(style, "edge_color_difference_threshold"),
                     .flat_shading = require_bool(style, "flat_shading"),
                     .point_color  = require_color(style, "point_rgba"),
                     .edge_color   = require_color(style, "edge_rgba"),
                     .face_palette = std::move(palette),
                     .camera       = {.projection = require_string(camera, "projection"),
                             .position   = require_vector(camera, "position"),
                             .target     = require_vector(camera, "target"),
                             .up         = require_vector(camera, "up"),
                             .vertical_field_of_view_radians = require_number(
                                 camera, "vertical_field_of_view_radians")},
                     .minimum_foreground_pixels = require_integer<std::size_t>(
                checks, "minimum_foreground_pixels")}
    };

    if (result.render.camera.projection != "perspective" &&
        result.render.camera.projection != "orthographic")
    {
      throw std::invalid_argument(
          "Camera projection must be 'perspective' or 'orthographic'.");
    }
    if (result.render.edge_scope != "all" &&
        result.render.edge_scope != "screen_space_face_boundaries")
    {
      throw std::invalid_argument(
          "Geometry edge scope must be 'all' or "
          "'screen_space_face_boundaries'.");
    }
    if (result.render.vertex_scope != "all" &&
        result.render.vertex_scope != "convex_hull")
    {
      throw std::invalid_argument(
          "Geometry vertex scope must be 'all' or 'convex_hull'.");
    }
    if (result.render.edge_color_difference_threshold < 0 ||
        result.render.edge_color_difference_threshold > 765)
    {
      throw std::invalid_argument(
          "Edge color-difference threshold must be from 0 through 765.");
    }
    if (result.render.camera.vertical_field_of_view_radians <= 0.0 ||
        result.render.camera.vertical_field_of_view_radians >= std::numbers::pi)
    {
      throw std::invalid_argument(
          "Camera field of view must be between zero and pi radians.");
    }
    return result;
  }

  [[nodiscard]] auto sha256(std::filesystem::path const& path) -> std::string
  {
    QFile file(QString::fromStdString(path.string()));
    if (!file.open(QIODevice::ReadOnly))
    {
      throw std::filesystem::filesystem_error(
          "Could not open fixture for hashing", path,
          std::make_error_code(std::errc::no_such_file_or_directory));
    }
    QCryptographicHash digest(QCryptographicHash::Sha256);
    if (!digest.addData(&file))
    {
      throw std::runtime_error("Could not hash the viewer fixture.");
    }
    return digest.result().toHex().toStdString();
  }

  [[nodiscard]] auto topology_counts(Delaunay const& triangulation)
      -> Topology_counts
  {
    if (!triangulation.is_valid() || triangulation.dimension() != 3)
    {
      throw std::invalid_argument(
          "The viewer fixture is not a valid three-dimensional "
          "triangulation.");
    }
    if (triangulation.number_of_vertices() == 0)
    {
      throw std::invalid_argument("The viewer fixture is empty.");
    }

    auto minimum = std::numeric_limits<cdt::Int_precision>::max();
    auto maximum = std::numeric_limits<cdt::Int_precision>::min();
    for (auto const vertex : triangulation.finite_vertex_handles())
    {
      minimum = std::min(minimum, vertex->info());
      maximum = std::max(maximum, vertex->info());
    }
    return {.vertices          = triangulation.number_of_vertices(),
            .edges             = triangulation.number_of_finite_edges(),
            .faces             = triangulation.number_of_finite_facets(),
            .simplices         = triangulation.number_of_finite_cells(),
            .minimum_timeslice = minimum,
            .maximum_timeslice = maximum};
  }

  void validate_fixture(Delaunay const&        triangulation,
                        Viewer_manifest const& manifest)
  {
    auto const actual = topology_counts(triangulation);
    if (actual.vertices != manifest.expected.vertices ||
        actual.edges != manifest.expected.edges ||
        actual.faces != manifest.expected.faces ||
        actual.simplices != manifest.expected.simplices ||
        actual.minimum_timeslice != manifest.expected.minimum_timeslice ||
        actual.maximum_timeslice != manifest.expected.maximum_timeslice)
    {
      throw std::invalid_argument(fmt::format(
          "Viewer fixture topology does not match the manifest: got "
          "V/E/F/T={}/{}/{}/{} and timeslices {}..{}.",
          actual.vertices, actual.edges, actual.faces, actual.simplices,
          actual.minimum_timeslice, actual.maximum_timeslice));
    }
  }

  constexpr auto FNV_OFFSET = std::uint64_t{14695981039346656037ULL};
  constexpr auto FNV_PRIME  = std::uint64_t{1099511628211ULL};

  [[nodiscard]] constexpr auto mix_hash(std::uint64_t hash) noexcept
      -> std::uint64_t
  {
    hash ^= hash >> 30U;
    hash *= 0xbf58476d1ce4e5b9ULL;
    hash ^= hash >> 27U;
    hash *= 0x94d049bb133111ebULL;
    return hash ^ (hash >> 31U);
  }

  void append_hash(std::uint64_t& hash, std::uint64_t value) noexcept
  {
    for (unsigned int byte = 0; byte < 8; ++byte)
    {
      hash ^= (value >> (byte * 8U)) & 0xffU;
      hash *= FNV_PRIME;
    }
  }

  [[nodiscard]] auto vertex_key(Triangulation::Vertex_handle const vertex)
      -> std::array<std::uint64_t, 4>
  {
    auto const point = vertex->point();
    return {std::bit_cast<std::uint64_t>(CGAL::to_double(point.x())),
            std::bit_cast<std::uint64_t>(CGAL::to_double(point.y())),
            std::bit_cast<std::uint64_t>(CGAL::to_double(point.z())),
            static_cast<std::uint64_t>(vertex->info())};
  }

  [[nodiscard]] auto facet_vertices(
      Triangulation::Finite_facets_iterator const facet)
      -> std::array<Triangulation::Vertex_handle, 3>
  {
    auto const                                  cell  = facet->first;
    auto const                                  index = facet->second;
    std::array<Triangulation::Vertex_handle, 3> vertices{
        cell->vertex((index + 1) % 4), cell->vertex((index + 2) % 4),
        cell->vertex((index + 3) % 4)};
    std::ranges::sort(vertices, {}, vertex_key);
    return vertices;
  }

  [[nodiscard]] auto facet_hash(
      std::array<Triangulation::Vertex_handle, 3> const& vertices)
      -> std::uint64_t
  {
    auto hash = FNV_OFFSET;
    for (auto const vertex : vertices)
    {
      for (auto const value : vertex_key(vertex)) { append_hash(hash, value); }
    }
    return mix_hash(hash);
  }

  [[nodiscard]] auto convex_hull_vertices(Triangulation const& triangulation)
      -> std::set<std::array<std::uint64_t, 4>>
  {
    std::set<std::array<std::uint64_t, 4>> result;
    for (auto facet = triangulation.finite_facets_begin();
         facet != triangulation.finite_facets_end(); ++facet)
    {
      auto const cell     = facet->first;
      auto const opposite = facet->second;
      if (!triangulation.is_infinite(cell) &&
          !triangulation.is_infinite(cell->neighbor(opposite)))
      {
        continue;
      }
      for (auto const vertex : facet_vertices(facet))
      {
        result.emplace(vertex_key(vertex));
      }
    }
    return result;
  }

  void orient_outward(
      std::array<Triangulation::Vertex_handle, 3>& vertices) noexcept
  {
    auto const point = [](Triangulation::Vertex_handle const vertex) {
      auto const& value = vertex->point();
      return std::array{CGAL::to_double(value.x()), CGAL::to_double(value.y()),
                        CGAL::to_double(value.z())};
    };
    auto const a  = point(vertices[0]);
    auto const b  = point(vertices[1]);
    auto const c  = point(vertices[2]);
    auto const ab = std::array{b[0] - a[0], b[1] - a[1], b[2] - a[2]};
    auto const ac = std::array{c[0] - a[0], c[1] - a[1], c[2] - a[2]};
    auto const normal =
        std::array{ab[1] * ac[2] - ab[2] * ac[1], ab[2] * ac[0] - ab[0] * ac[2],
                   ab[0] * ac[1] - ab[1] * ac[0]};
    auto const centroid =
        std::array{a[0] + b[0] + c[0], a[1] + b[1] + c[1], a[2] + b[2] + c[2]};
    auto const radial_alignment = normal[0] * centroid[0] +
                                  normal[1] * centroid[1] +
                                  normal[2] * centroid[2];
    if (radial_alignment < 0.0) { std::swap(vertices[1], vertices[2]); }
  }

  [[nodiscard]] auto make_scene(Delaunay const&      delaunay,
                                Render_config const& render)
      -> CGAL::Graphics_scene
  {
    auto const&   triangulation = static_cast<Triangulation const&>(delaunay);
    auto const    hull_vertices = render.vertex_scope == "convex_hull"
                                    ? convex_hull_vertices(triangulation)
                                    : std::set<std::array<std::uint64_t, 4>>{};
    Scene_options options;
    options.ignore_all_vertices(!render.draw_vertices);
    options.ignore_all_edges(!render.draw_edges || render.edge_scope != "all");
    options.ignore_all_faces(true);
    options.colored_vertex = [](Triangulation const&,
                                Triangulation::Vertex_handle) { return true; };
    options.draw_vertex    = [&hull_vertices, &render](
                                 Triangulation const&,
                                 Triangulation::Vertex_handle vertex) {
      return render.vertex_scope == "all" ||
             hull_vertices.contains(vertex_key(vertex));
    };
    options.vertex_color = [&render](Triangulation const&,
                                     Triangulation::Vertex_handle) {
      return to_cgal_color(render.point_color);
    };
    options.colored_edge = [](Triangulation const&,
                              Triangulation::Finite_edges_iterator) {
      return true;
    };
    options.edge_color = [&render](Triangulation const&,
                                   Triangulation::Finite_edges_iterator) {
      return to_cgal_color(render.edge_color);
    };
    CGAL::Graphics_scene scene;
    CGAL::add_to_graphics_scene(triangulation, scene, options);
    if (render.draw_faces)
    {
      for (auto facet = triangulation.finite_facets_begin();
           facet != triangulation.finite_facets_end(); ++facet)
      {
        auto       vertices = facet_vertices(facet);
        auto const index = static_cast<std::size_t>(facet_hash(vertices) %
                                                    render.face_palette.size());
        orient_outward(vertices);
        scene.face_begin(render.face_palette.at(index));
        for (auto const vertex : vertices)
        {
          scene.add_point_in_face(vertex->point());
        }
        scene.face_end();
      }
    }
    return scene;
  }

  [[nodiscard]] auto vec(std::array<double, 3> const& values)
      -> CGAL::qglviewer::Vec
  { return {values[0], values[1], values[2]}; }

  void configure_viewer(CGAL::Qt::Basic_viewer& viewer,
                        Render_config const&    render)
  {
    viewer.resize(render.width, render.height);
    viewer.draw_vertices(render.draw_vertices);
    viewer.draw_edges(render.draw_edges && render.edge_scope == "all");
    viewer.draw_faces(render.draw_faces);
    viewer.size_vertices(render.point_size);
    viewer.size_edges(render.line_width);
    viewer.flat_shading(render.flat_shading);
    viewer.setBackgroundColor(render.background);

    auto* const camera = viewer.camera();
    camera->setType(render.camera.projection == "perspective"
                        ? CGAL::qglviewer::Camera::PERSPECTIVE
                        : CGAL::qglviewer::Camera::ORTHOGRAPHIC);
    camera->setFieldOfView(render.camera.vertical_field_of_view_radians);
    camera->setPosition(vec(render.camera.position));
    camera->setUpVector(vec(render.camera.up));
    camera->lookAt(vec(render.camera.target));
    viewer.redraw();
  }

  [[nodiscard]] auto color_distance(QColor const& first,
                                    QColor const& second) noexcept -> int
  {
    return std::abs(first.red() - second.red()) +
           std::abs(first.green() - second.green()) +
           std::abs(first.blue() - second.blue());
  }

  void outline_face_boundaries(std::filesystem::path const& path,
                               Render_config const&         render)
  {
    if (!render.draw_edges ||
        render.edge_scope != "screen_space_face_boundaries")
    {
      return;
    }

    QImage source(QString::fromStdString(path.string()));
    if (source.isNull())
    {
      throw std::runtime_error(
          "Could not reopen the rendered image for edge outlining.");
    }
    source        = source.convertToFormat(QImage::Format_RGBA8888);
    auto outlined = source;

    for (int y = 0; y < source.height(); ++y)
    {
      for (int x = 0; x < source.width(); ++x)
      {
        auto const color = source.pixelColor(x, y);
        if (color.alpha() == 0) { continue; }
        auto const boundary_with = [&](int neighbor_x, int neighbor_y) {
          if (neighbor_x >= source.width() || neighbor_y >= source.height())
          {
            return false;
          }
          auto const neighbor = source.pixelColor(neighbor_x, neighbor_y);
          return neighbor.alpha() != 0 &&
                 color_distance(color, neighbor) >
                     render.edge_color_difference_threshold;
        };
        auto const right_boundary = boundary_with(x + 1, y);
        auto const lower_boundary = boundary_with(x, y + 1);
        if (right_boundary || lower_boundary)
        {
          outlined.setPixelColor(x, y, render.edge_color);
          if (render.line_width > 1.0F)
          {
            if (right_boundary)
            {
              outlined.setPixelColor(x + 1, y, render.edge_color);
            }
            if (lower_boundary)
            {
              outlined.setPixelColor(x, y + 1, render.edge_color);
            }
          }
        }
      }
    }

    auto const is_point = [&render](QColor const& color) {
      return color.alpha() != 0 &&
             color_distance(color, render.point_color) <= 6;
    };
    for (int y = 0; y < source.height(); ++y)
    {
      for (int x = 0; x < source.width(); ++x)
      {
        if (!is_point(source.pixelColor(x, y))) { continue; }
        auto const has_point_interior = x > 0 && x + 1 < source.width() &&
                                        y > 0 && y + 1 < source.height() &&
                                        is_point(source.pixelColor(x - 1, y)) &&
                                        is_point(source.pixelColor(x + 1, y)) &&
                                        is_point(source.pixelColor(x, y - 1)) &&
                                        is_point(source.pixelColor(x, y + 1));
        if (!has_point_interior)
        {
          outlined.setPixelColor(x, y, render.edge_color);
        }
      }
    }

    if (!outlined.save(QString::fromStdString(path.string()), "PNG"))
    {
      throw std::runtime_error("Could not save the outlined render.");
    }
  }

  [[nodiscard]] auto foreground_pixels(QImage               image,
                                       Render_config const& render)
      -> std::size_t
  {
    image = image.convertToFormat(QImage::Format_RGBA8888);
    std::size_t count{};
    for (int y = 0; y < image.height(); ++y)
    {
      for (int x = 0; x < image.width(); ++x)
      {
        auto const color = image.pixelColor(x, y);
        if (render.transparent_background ? color.alpha() != 0
                                          : color != render.background)
        {
          ++count;
        }
      }
    }
    return count;
  }

  void validate_image(std::filesystem::path const& path,
                      Render_config const&         render)
  {
    QImageReader reader(QString::fromStdString(path.string()));
    if (!reader.canRead())
    {
      throw std::runtime_error(fmt::format(
          "Renderer did not produce a readable image at {}.", path.string()));
    }
    auto const dimensions = reader.size();
    if (dimensions.width() != render.width ||
        dimensions.height() != render.height)
    {
      throw std::runtime_error(
          fmt::format("Rendered image has dimensions {}x{}, expected {}x{}.",
                      dimensions.width(), dimensions.height(), render.width,
                      render.height));
    }
    auto image = reader.read();
    if (image.isNull())
    {
      throw std::runtime_error("Renderer produced an unreadable image.");
    }
    auto const foreground = foreground_pixels(std::move(image), render);
    if (foreground < render.minimum_foreground_pixels)
    {
      throw std::runtime_error(fmt::format(
          "Rendered image contains {} foreground pixels; expected at least "
          "{}.",
          foreground, render.minimum_foreground_pixels));
    }
  }

  [[nodiscard]] auto run_viewer(CGAL::Graphics_scene const&  scene,
                                Render_config const&         render,
                                std::filesystem::path const& output) -> int
  {
    CGAL::Qt::init_ogl_context(4, 3);
    int             qt_argc            = 1;
    char            application_name[] = "cdt-viewer";
    char*           qt_argv[]          = {application_name, nullptr};
    QApplication    application(qt_argc, qt_argv);
    Artifact_viewer viewer(nullptr, scene, "CDT++ archival viewer",
                           render.draw_vertices, render.draw_edges,
                           render.draw_faces);

    std::string     render_error;
    if (!output.empty())
    {
      viewer.setAttribute(::Qt::WA_DontShowOnScreen, true);
      viewer.after_initialization(
          [&application, &viewer, &render, &output, &render_error]() {
            try
            {
              configure_viewer(viewer, render);
              if (output.has_parent_path())
              {
                std::filesystem::create_directories(output.parent_path());
              }
              viewer.saveSnapshot(QString::fromStdString(output.string()),
                                  render.width, render.height,
                                  render.expand_frustum, render.oversampling,
                                  render.transparent_background
                                      ? CGAL::qglviewer::TRANSPARENT_BACKGROUND
                                      : CGAL::qglviewer::CURRENT_BACKGROUND);
              outline_face_boundaries(output, render);
              validate_image(output, render);
              application.exit(EXIT_SUCCESS);
            }
            catch (std::exception const& error)
            {
              render_error = error.what();
              application.exit(EXIT_FAILURE);
            }
          });
    }
    else
    {
      viewer.after_initialization(
          [&viewer, &render]() { configure_viewer(viewer, render); });
    }

    viewer.show();
    auto const status = application.exec();
    if (!render_error.empty()) { throw std::runtime_error(render_error); }
    return status;
  }
}  // namespace

auto main(int const argc, char* const argv[]) -> int
try
{
  std::string             manifest_path;
  std::string             fixture_override;
  std::string             output_path;

  po::options_description description(std::string{USAGE});
  description.add_options()("help,h", "Show this message")(
      "version,v", "Show program version")(
      "manifest,m", po::value<std::string>(&manifest_path)->required(),
      "Versioned JSON render manifest")(
      "fixture,f", po::value<std::string>(&fixture_override),
      "Override the manifest fixture path (digest and topology still apply)")(
      "output,o", po::value<std::string>(&output_path),
      "Render noninteractively to this image and exit");

  po::variables_map arguments;
  po::store(po::parse_command_line(argc, argv, description), arguments);
  if (arguments.count("help") != 0U)
  {
    fmt::print("{}\n", fmt::streamed(description));
    return EXIT_SUCCESS;
  }
  if (arguments.count("version") != 0U)
  {
    fmt::print("cdt-viewer version {} (CGAL {}, Qt {})\n", cdt::VERSION,
               CGAL_VERSION_STR, qVersion());
    return EXIT_SUCCESS;
  }
  po::notify(arguments);

  auto manifest = parse_manifest(manifest_path);
  if (!fixture_override.empty())
  {
    manifest.fixture = std::filesystem::weakly_canonical(fixture_override);
  }
  if (sha256(manifest.fixture) != manifest.fixture_sha256)
  {
    throw std::invalid_argument(
        "Viewer fixture SHA-256 does not match the render manifest.");
  }
  if (manifest.cgal_version != CGAL_VERSION_STR ||
      manifest.qt_version != qVersion())
  {
    throw std::runtime_error(fmt::format(
        "Renderer version mismatch: manifest requires CGAL {} and Qt {}; "
        "this binary uses CGAL {} and Qt {}.",
        manifest.cgal_version, manifest.qt_version, CGAL_VERSION_STR,
        qVersion()));
  }

  auto const triangulation =
      cdt::utilities::read_file<Delaunay>(manifest.fixture);
  validate_fixture(triangulation, manifest);
  auto const scene = make_scene(triangulation, manifest.render);

  fmt::print("Validated viewer fixture {} with V/E/F/T={}/{}/{}/{}.\n",
             manifest.fixture.string(), manifest.expected.vertices,
             manifest.expected.edges, manifest.expected.faces,
             manifest.expected.simplices);
  auto const output = output_path.empty() ? std::filesystem::path{}
                                          : std::filesystem::path{output_path};
  auto const status = run_viewer(scene, manifest.render, output);
  if (!output.empty())
  {
    fmt::print("Rendered {}x{} artifact to {}.\n", manifest.render.width,
               manifest.render.height, output.string());
  }
  return status;
}
catch (std::exception const& error)
{
  fmt::print(stderr, "cdt-viewer: {}\n", error.what());
  return EXIT_FAILURE;
}
catch (...)
{
  fmt::print(stderr, "cdt-viewer: unknown failure\n");
  return EXIT_FAILURE;
}
