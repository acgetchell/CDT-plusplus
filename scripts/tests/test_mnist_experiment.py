"""Dependency-free tests for the PyTorch MNIST command boundary."""

import argparse
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

from scripts.mnist_experiment import (
    MAX_TORCH_SEED,
    MIN_TORCH_SEED,
    _config_from_args,
    _parse_args,
    _parse_seed,
    _positive_float,
    _positive_int,
    _staged_run_directory,
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
            self.assertEqual(list(root.glob(".run.incomplete-*")), [])

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


if __name__ == "__main__":
    unittest.main()
