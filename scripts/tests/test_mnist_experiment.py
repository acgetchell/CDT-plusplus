"""Dependency-free tests for the PyTorch MNIST command boundary."""

import argparse
import unittest
from contextlib import redirect_stderr
from io import StringIO
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest.mock import Mock, patch

from scripts.mnist_experiment import (
    MAX_TORCH_SEED,
    MIN_TORCH_SEED,
    _config_from_args,
    _mirror_to_comet,
    _parse_args,
    _parse_seed,
    _positive_float,
    _positive_int,
    _staged_run_directory,
    _start_optional_comet,
    main,
)


class MnistExperimentTests(unittest.TestCase):
    """Verify configuration parsing before optional dependencies load."""

    def test_defaults_preserve_the_cpu_portability_example(self) -> None:
        """The retained MNIST command is deterministic, local-first, and opt-in."""
        config = _config_from_args(_parse_args([]))
        self.assertEqual(config.seed, 0)
        self.assertEqual(config.comet_mode, "disabled")
        self.assertTrue(config.download)
        self.assertEqual(config.data_directory, Path("out/experiments/data").resolve())
        self.assertEqual(config.output_directory, Path("out/experiments/mnist").resolve())

    def test_no_download_requires_existing_local_inputs(self) -> None:
        """A caller can prohibit network-backed dataset acquisition."""
        self.assertFalse(_config_from_args(_parse_args(["--no-download"])).download)

    def test_data_and_output_paths_must_not_overlap_retained_inputs(self) -> None:
        """Invalid path containment fails before creating or importing anything."""
        with TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            cases = (
                (root / "output" / "data", root / "output", "data directory must not be inside the output directory"),
                (root / "data", root / "data" / "MNIST" / "runs" / "run", "output directory must not be inside the retained MNIST dataset directory"),
            )
            for data_directory, output_directory, expected_message in cases:
                with self.subTest(data_directory=data_directory, output_directory=output_directory):
                    stderr = StringIO()
                    with redirect_stderr(stderr):
                        status = main(
                            [
                                "--data-directory",
                                str(data_directory),
                                "--output-directory",
                                str(output_directory),
                            ]
                        )

                    self.assertEqual(status, 2)
                    self.assertIn(expected_message, stderr.getvalue())
                    self.assertEqual(list(root.iterdir()), [])

    def test_output_may_be_inside_data_directory_outside_retained_inputs(self) -> None:
        """Run artifacts may share a parent with, but cannot enter, MNIST inputs."""
        with TemporaryDirectory() as temporary_directory:
            data_directory = Path(temporary_directory) / "data"
            output_directory = data_directory / "runs" / "run"

            config = _config_from_args(
                _parse_args(
                    [
                        "--data-directory",
                        str(data_directory),
                        "--output-directory",
                        str(output_directory),
                    ]
                )
            )

            self.assertEqual(config.data_directory, data_directory.resolve())
            self.assertEqual(config.output_directory, output_directory.resolve())

    def test_training_counts_must_be_positive_and_finite(self) -> None:
        """Invalid hyperparameters fail before Torch or Comet is imported."""
        with self.assertRaises(argparse.ArgumentTypeError):
            _positive_int("0")
        for value in ("0", "nan", "inf", "-inf"):
            with self.subTest(value=value), self.assertRaises(argparse.ArgumentTypeError):
                _positive_float(value)

    def test_seed_must_fit_the_pytorch_generator_range(self) -> None:
        """Every accepted seed can be forwarded to both PyTorch generators."""
        self.assertEqual(_parse_seed(str(MIN_TORCH_SEED)), MIN_TORCH_SEED)
        self.assertEqual(_parse_seed(str(MAX_TORCH_SEED)), MAX_TORCH_SEED)
        for value in (str(MIN_TORCH_SEED - 1), str(MAX_TORCH_SEED + 1)):
            with self.subTest(value=value), self.assertRaises(argparse.ArgumentTypeError):
                _parse_seed(value)

    def test_comet_start_failure_does_not_block_the_local_run(self) -> None:
        """An unavailable optional service is reported and then omitted."""
        config = _config_from_args(_parse_args(["--comet", "offline"]))
        stderr = StringIO()
        with (
            patch("scripts.mnist_experiment._start_comet", side_effect=RuntimeError("Comet unavailable")),
            redirect_stderr(stderr),
        ):
            comet_run = _start_optional_comet(config, config.output_directory)

        self.assertIsNone(comet_run)
        self.assertIn("Comet mirror failed while starting the experiment", stderr.getvalue())

    def test_comet_operation_failure_does_not_escape(self) -> None:
        """A late optional mirror failure cannot replace a local exception."""
        operation = Mock(side_effect=RuntimeError("Comet unavailable"))
        stderr = StringIO()
        with redirect_stderr(stderr):
            _mirror_to_comet("ending the experiment", operation)

        operation.assert_called_once_with()
        self.assertIn("Comet mirror failed while ending the experiment", stderr.getvalue())

    def test_completed_run_directory_is_published_once(self) -> None:
        """A successful generation appears only after its contents are complete."""
        with TemporaryDirectory() as temporary_directory:
            output_directory = Path(temporary_directory) / "run"
            with _staged_run_directory(output_directory) as staging_directory:
                (staging_directory / "run.json").write_text("complete\n", encoding="utf-8")
                self.assertFalse(output_directory.exists())

            self.assertEqual((output_directory / "run.json").read_text(encoding="utf-8"), "complete\n")

    def test_failed_run_does_not_publish_partial_artifacts(self) -> None:
        """A failed generation leaves neither a final nor an incomplete run."""
        with TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            output_directory = root / "run"
            message = "training failed"
            with self.assertRaisesRegex(RuntimeError, message), _staged_run_directory(output_directory) as staging_directory:
                (staging_directory / "configuration.json").write_text("partial\n", encoding="utf-8")
                raise RuntimeError(message)

            self.assertFalse(output_directory.exists())
            self.assertEqual(list(root.iterdir()), [])

    def test_existing_run_is_never_overwritten(self) -> None:
        """A caller must choose a new output path for every generation."""
        with TemporaryDirectory() as temporary_directory:
            output_directory = Path(temporary_directory) / "run"
            output_directory.mkdir()
            sentinel = output_directory / "run.json"
            sentinel.write_text("previous\n", encoding="utf-8")

            with self.assertRaisesRegex(ValueError, "already exists"), _staged_run_directory(output_directory):
                self.fail("existing output directory was opened for replacement")

            self.assertEqual(sentinel.read_text(encoding="utf-8"), "previous\n")

    def test_unrelated_value_errors_propagate_from_the_experiment(self) -> None:
        """Serialization defects are not reported as configuration failures."""
        message = "configuration is not JSON serializable"
        with patch("scripts.mnist_experiment._run_experiment", side_effect=ValueError(message)), self.assertRaisesRegex(ValueError, message):
            main([])


if __name__ == "__main__":
    unittest.main()
