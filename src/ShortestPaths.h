/// Causal Dynamical Triangulations in C++ using CGAL
///
/// Copyright (c) 2015 Adam Getchell
///
/// Performs shortest paths algorithms on graphs using Bellman-Ford Algorithm
/// and Voronoi Diagrams
/// For details see:
/// ADD REFERENCES
/// ADD REFERENCES

/// \todo Graph Initialization
/// \todo AddVertex()
/// \todo AddEdge()
/// \todo IsNegativeCycle()
/// \todo CalculateBellmanFord()
/// \todo CalculateVoronoi()

/// @file ShortestPaths.h
/// @brief Calculate shortest path of a graph using Bellman Ford Algorithm and
/// Voronoi Diagrams
/// @author Gaurav Nagar

#ifndef SRC_SHORTESTPATHS_H_
#define SRC_SHORTESTPATHS_H_

// C++ headers
#include <algorithm>
#include <limits>
#include <map>
#include <tuple>
#include <vector>

const double inf =
    std::numerical_limits<double>::infinity();  // constant used to signify no
                                                // edge between two nodes

class Vertex
{
 private:
  double x_, y_, z_;
  double sourceDistance_;
  int    prev_;  // index of previous vertex

 public:
  Vertex() { Vertex(0, 0, 0); }

  Vertex(double x, double y, double z)
  {
    x_              = x;
    y_              = y;
    z_              = z;
    sourceDistance_ = std::numerical_limits<double>::infinity();
  }

  double getDistance() { return sourceDistance_; }

  int getPrev() { return prev_; }

  void setDistance(double distance) { sourceDistance_ = distance; }

  void setPrev(int prevVertex) { prev_ = prevVertex; }
};

class Edge
{
 private:
  Vertex u_, v_;   // indices of vertex u, v
  double weight_;  // weight of the edge

 public:
  Edge() { Edge(0, 1, 0.0); }

  Edge(Vertex u, Vertex v, double weight)
  {
    u_      = u;
    v_      = v;
    weight_ = weight;
  }

  Vertex getU() { return u_; }

  Vertex getV() { return v_; }

  double getWeight() { return weight_; }

  void setU(Vertex u) { u_ = u; }

  void setV(Vertex v) { v_ = u; }

  void setWeight(double weight) { weight_ = weight; }
};

/// @class Graph
///
/// @brief Graph class that provides shortest paths functionalities
///
/// The Bellman-Ford algorithm is a method that calculates the shortest paths on
/// graphs with even negative weights.
/// The reccurrence for the shortest paths is:
///
/// \D[i, j] = 0                                       if i = t,  j = 0
///            inf                                     if i != t, j = 0
///            min{D[k, j - 1] + w[i, k], D[i, j - 1]} if (i, k) is an edge

class Graph
{
 private:
  vector<Vertex>         vertices_;
  int                    numVertices_;
  int                    numEdges_;
  vector<Edge>           edges_;
  vector<vector<double>> adjMatrix_;
  vector<Vertex>         path_;
  map<Vertex : int>      verticesMap_;

 public:
  Graph(vector<Vertex> vertices, vector<Edge> edges)
  {
    vertices_    = vertices;
    edges_       = edges;
    numVertices_ = vertices.size();
    numEdges_    = edge.size();
    for (int i = 0; i < numVertices_; i++)
    {
      map[vertices_.at(i)] = i;
    }
    computeAdjMatrix();
  }

  void computeAdjMatrix()
  {
    for (int i = 0; i < numVertices_; i++)
    {
      vertex<int> adjList;  // adjList for ith vertex
      for (int j = 0; j < numVertices_; j++)
      {
        adjList.push_back(inf);
      }
      adjMatrix_.push_back(adjList);
    }

    for (int i = 0; i < numEdges_; i++)
    {
      int uIndex = verticesMap_[edges_.at(i).getU()];
      int vIndex = verticesMap_[edges_.at(i).getV()];
      adjMatrix_.at(uIndex).at(vIndex) =
          edges_.at(i).getWeight();  // edge weight between vertices u and v
      adjMatrix_.at(vIndex).at(uIndex) = edges_.at(i).getWeight();
    }
  }

  bool isNegativeCycle() { return true; }

  void calculateBellmanFord(Vertex s, Vertex t)
  {
    for (int i = 0; i < numVertices_; i++)
    {
      for (int j = 0; j < numVertices_; j++)
      {
        for (int k = 0; k < numVertices_; k++)
        {
        }
      }
    }
  }

  double calculateOptimalPathCost(Vertex s, Vertex t) { return 0; }

  vector<Vertex> calculateOptimalPath(Vertex s, Vertex t) { return path_; }
};

#endif  // SRC_SHORTESTPATHS_H_
