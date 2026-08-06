"""Run the CDT++ CPU-portable PyTorch MNIST experiment."""

import argparse
import math
import os
import platform
import sys
from dataclasses import asdict, dataclass
from importlib.metadata import PackageNotFoundError, version
from pathlib import Path
from typing import TYPE_CHECKING, Literal, Protocol, cast

from scripts.experiment_artifacts import (
    PACKAGE_NAME,
    OutputDirectoryExistsError,
    artifact_record,
    sha256,
    staged_run_directory,
    write_json,
)

if TYPE_CHECKING:
    from collections.abc import Callable, Mapping, Sequence
    from types import ModuleType

    from torch import Tensor
    from torch.nn import CrossEntropyLoss, Module
    from torch.optim import Optimizer
    from torch.utils.data import DataLoader

MIN_TORCH_SEED = -(1 << 63)
MAX_TORCH_SEED = (1 << 64) - 1


class ExperimentConfigurationError(ValueError):
    """The requested MNIST experiment configuration cannot be run."""


@dataclass(frozen=True)
class _RunConfig:
    """Inputs needed to reproduce one local MNIST run."""

    batch_size: int
    comet_mode: Literal["disabled", "offline", "online"]
    data_directory: Path
    download: bool
    epochs: int
    learning_rate: float
    output_directory: Path
    seed: int


class _Experiment(Protocol):
    """Comet operations mirrored by the PyTorch experiment."""

    def log_parameters(self, parameters: Mapping[str, object]) -> object:
        """Record the complete declared training configuration."""
        ...

    def log_metric(self, name: str, value: float, *, epoch: int | None = None) -> object:
        """Record one training or evaluation metric."""
        ...

    def end(self) -> object:
        """Flush and close the Comet run."""
        ...


class _Watch(Protocol):
    """Callable interface for Comet's PyTorch parameter watcher."""

    def __call__(self, model: object) -> object:
        """Register a model for weight, bias, and gradient logging."""
        ...


class _LogModel(Protocol):
    """Callable interface for Comet's PyTorch checkpoint logger."""

    def __call__(self, experiment: _Experiment, model: object, *, model_name: str) -> object:
        """Store a model or training checkpoint in Comet."""
        ...


@dataclass(frozen=True)
class _CometRun:
    """Comet integration functions loaded before PyTorch."""

    experiment: _Experiment
    log_model: _LogModel
    watch: _Watch


def _mirror_to_comet(action: str, operation: Callable[..., object], /, *args: object, **kwargs: object) -> None:
    """Attempt one optional Comet operation without invalidating local output."""
    try:
        operation(*args, **kwargs)
    except Exception as error:  # noqa: BLE001 - The optional mirror cannot invalidate canonical local artifacts.
        print(f"Comet mirror failed while {action}: {error}", file=sys.stderr)


def _start_optional_comet(config: _RunConfig, artifact_directory: Path) -> _CometRun | None:
    """Start the optional Comet mirror without making it a local-run dependency."""
    try:
        return _start_comet(config, artifact_directory)
    except ExperimentConfigurationError, ModuleNotFoundError:
        raise
    except Exception as error:  # noqa: BLE001 - The optional mirror cannot invalidate canonical local artifacts.
        print(f"Comet mirror failed while starting the experiment: {error}", file=sys.stderr)
        return None


def _positive_int(value: str) -> int:
    """Parse a strictly positive integer command-line value."""
    parsed = int(value, 10)
    if parsed <= 0:
        message = "value must be greater than zero"
        raise argparse.ArgumentTypeError(message)
    return parsed


def _positive_float(value: str) -> float:
    """Parse a strictly positive floating-point command-line value."""
    parsed = float(value)
    if not math.isfinite(parsed) or parsed <= 0.0:
        message = "value must be finite and greater than zero"
        raise argparse.ArgumentTypeError(message)
    return parsed


def _parse_seed(value: str) -> int:
    """Parse a seed accepted by both PyTorch CPU generators."""
    try:
        seed = int(value, 10)
    except ValueError as error:
        message = "seed must be an integer"
        raise argparse.ArgumentTypeError(message) from error
    if seed < MIN_TORCH_SEED or seed > MAX_TORCH_SEED:
        message = f"seed must be between {MIN_TORCH_SEED} and {MAX_TORCH_SEED}"
        raise argparse.ArgumentTypeError(message)
    return seed


def _parse_args(argv: Sequence[str]) -> argparse.Namespace:
    """Parse command-line arguments without loading experiment dependencies."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--batch-size", type=_positive_int, default=64, help="training and evaluation batch size (default: 64)")
    parser.add_argument(
        "--comet",
        choices=("disabled", "offline", "online"),
        default="disabled",
        help="optional Comet mirror; local artifacts are always canonical (default: disabled)",
    )
    parser.add_argument(
        "--data-directory",
        type=Path,
        default=Path("out/experiments/data"),
        help="persistent local MNIST input directory (default: out/experiments/data)",
    )
    parser.add_argument("--epochs", type=_positive_int, default=5, help="training epochs (default: 5)")
    parser.add_argument("--learning-rate", type=_positive_float, default=0.001, help="Adam learning rate (default: 0.001)")
    parser.add_argument("--no-download", action="store_false", dest="download", help="require MNIST to exist in the local data directory")
    parser.add_argument(
        "--output-directory",
        type=Path,
        default=Path("out/experiments/mnist"),
        help="canonical local run-artifact directory (default: out/experiments/mnist)",
    )
    parser.add_argument("--seed", type=_parse_seed, default=0, help="PyTorch and data-loader seed (default: 0)")
    return parser.parse_args(argv)


def _config_from_args(args: argparse.Namespace) -> _RunConfig:
    """Convert parsed arguments into the immutable run configuration."""
    data_directory = args.data_directory.resolve()
    output_directory = args.output_directory.resolve()
    dataset_directory = (data_directory / "MNIST").resolve()
    if data_directory.is_relative_to(output_directory):
        message = "MNIST data directory must not be inside the output directory."
        raise ExperimentConfigurationError(message)
    if output_directory.is_relative_to(dataset_directory):
        message = "MNIST output directory must not be inside the retained MNIST dataset directory."
        raise ExperimentConfigurationError(message)
    return _RunConfig(
        batch_size=args.batch_size,
        comet_mode=args.comet,
        data_directory=data_directory,
        download=args.download,
        epochs=args.epochs,
        learning_rate=args.learning_rate,
        output_directory=output_directory,
        seed=args.seed,
    )


def _configuration_payload(config: _RunConfig, *, torch_version: str | None = None, torchvision_version: str | None = None) -> dict[str, object]:
    """Return a JSON-safe configuration and runtime record."""
    payload = asdict(config)
    payload["data_directory"] = str(config.data_directory)
    payload["output_directory"] = str(config.output_directory)
    payload["device"] = "cpu"
    payload["package"] = PACKAGE_NAME
    try:
        payload["package_version"] = version(PACKAGE_NAME)
    except PackageNotFoundError:
        payload["package_version"] = "uninstalled"
    payload["platform"] = platform.platform()
    payload["python"] = platform.python_version()
    payload["script_sha256"] = sha256(Path(__file__))
    if torch_version is not None:
        payload["torch"] = torch_version
    if torchvision_version is not None:
        payload["torchvision"] = torchvision_version
    return payload


def _dataset_manifest(dataset_root: Path) -> list[dict[str, object]]:
    """Describe every retained local MNIST input file."""
    return [
        {
            "bytes": path.stat().st_size,
            "path": path.relative_to(dataset_root).as_posix(),
            "sha256": sha256(path),
        }
        for path in sorted(
            (path for path in dataset_root.rglob("*") if path.is_file()),
            key=lambda path: path.relative_to(dataset_root).as_posix(),
        )
    ]


def _start_comet(config: _RunConfig, artifact_directory: Path) -> _CometRun:
    """Start Comet before importing PyTorch so automatic logging can attach."""
    api_key = None
    if config.comet_mode == "online":
        api_key = os.environ.get("COMET_API_KEY")
        if not api_key:
            message = "COMET_API_KEY is required when --comet online is selected."
            raise ExperimentConfigurationError(message)

    import comet_ml  # noqa: PLC0415

    if config.comet_mode == "online":
        experiment_config = comet_ml.ExperimentConfig(
            auto_histogram_gradient_logging=True,
            auto_histogram_weight_logging=True,
            log_env_details=False,
            log_env_network=False,
            log_git_metadata=False,
            log_git_patch=False,
            log_graph=True,
            name="cdt-mnist",
        )
        experiment = comet_ml.start(api_key=api_key, experiment_config=experiment_config, mode="create", project_name="cdt-plusplus")
    else:
        offline_directory = artifact_directory / "comet"
        offline_directory.mkdir(parents=True, exist_ok=True)
        experiment_config = comet_ml.ExperimentConfig(
            auto_histogram_gradient_logging=True,
            auto_histogram_weight_logging=True,
            log_env_details=False,
            log_env_network=False,
            log_git_metadata=False,
            log_git_patch=False,
            log_graph=True,
            name="cdt-mnist",
            offline_directory=str(offline_directory),
        )
        experiment = comet_ml.start(
            experiment_config=experiment_config,
            mode="create",
            online=False,
            project_name="cdt-plusplus",
        )

    from comet_ml.integration.pytorch import log_model, watch  # noqa: PLC0415

    return _CometRun(
        experiment=cast("_Experiment", experiment),
        log_model=cast("_LogModel", log_model),
        watch=cast("_Watch", watch),
    )


def _build_model(torch_module: ModuleType) -> Module:
    """Build the historical dense MNIST architecture with PyTorch."""
    return torch_module.nn.Sequential(
        torch_module.nn.Flatten(),
        torch_module.nn.Linear(28 * 28, 128),
        torch_module.nn.ReLU(),
        torch_module.nn.Dropout(0.2),
        torch_module.nn.Linear(128, 10),
    )


def _train_epoch(
    model: Module,
    data_loader: DataLoader[tuple[Tensor, ...]],
    loss_function: CrossEntropyLoss,
    optimizer: Optimizer,
) -> float:
    """Train one deterministic CPU epoch and return mean loss."""
    model.train()
    total_loss = 0.0
    total_examples = 0
    for images, labels in data_loader:
        optimizer.zero_grad()
        logits = model(images)
        loss = loss_function(logits, labels)
        loss.backward()
        optimizer.step()
        examples = int(labels.size(0))
        total_examples += examples
        total_loss += float(loss.item()) * examples
    return total_loss / total_examples


def _evaluate(
    torch_module: ModuleType,
    model: Module,
    data_loader: DataLoader[tuple[Tensor, ...]],
    loss_function: CrossEntropyLoss,
) -> tuple[float, float]:
    """Evaluate mean loss and accuracy on the deterministic CPU baseline."""
    model.eval()
    total_loss = 0.0
    total_examples = 0
    correct = 0
    with torch_module.no_grad():
        for images, labels in data_loader:
            logits = model(images)
            loss = loss_function(logits, labels)
            examples = int(labels.size(0))
            total_examples += examples
            total_loss += float(loss.item()) * examples
            correct += int((logits.argmax(dim=1) == labels).sum().item())
    return total_loss / total_examples, correct / total_examples


def _run_experiment(config: _RunConfig) -> None:  # noqa: PLR0915 - Keep the staged experiment lifecycle together.
    """Train PyTorch on MNIST and retain a complete local run record."""
    with staged_run_directory(config.output_directory) as artifact_directory:
        configuration_path = artifact_directory / "configuration.json"

        comet_run = _start_optional_comet(config, artifact_directory) if config.comet_mode != "disabled" else None
        try:
            import torch  # noqa: PLC0415
            from torchvision import datasets, transforms  # noqa: PLC0415

            torch.manual_seed(config.seed)
            torch.use_deterministic_algorithms(mode=True)
            generator = torch.Generator().manual_seed(config.seed)
            transform = transforms.ToTensor()
            try:
                training_data = datasets.MNIST(root=config.data_directory, train=True, download=config.download, transform=transform)
                test_data = datasets.MNIST(root=config.data_directory, train=False, download=config.download, transform=transform)
            except RuntimeError as error:
                message = f"MNIST data is unavailable or incomplete at {config.data_directory}: {error}"
                raise ExperimentConfigurationError(message) from error
            training_loader = torch.utils.data.DataLoader(
                training_data,
                batch_size=config.batch_size,
                generator=generator,
                num_workers=0,
                shuffle=True,
            )
            test_loader = torch.utils.data.DataLoader(test_data, batch_size=config.batch_size, num_workers=0, shuffle=False)

            model = _build_model(torch)
            loss_function = torch.nn.CrossEntropyLoss()
            optimizer = torch.optim.Adam(model.parameters(), lr=config.learning_rate)
            configuration = _configuration_payload(config, torch_version=torch.__version__, torchvision_version=version("torchvision"))
            write_json(configuration_path, configuration)

            if comet_run is not None:
                _mirror_to_comet("logging parameters", comet_run.experiment.log_parameters, configuration)
                _mirror_to_comet("watching the model", comet_run.watch, model)

            training_metrics: list[dict[str, object]] = []
            for epoch in range(1, config.epochs + 1):
                training_loss = _train_epoch(model, training_loader, loss_function, optimizer)
                training_metrics.append({"epoch": epoch, "loss": training_loss})
                if comet_run is not None:
                    _mirror_to_comet("logging training loss", comet_run.experiment.log_metric, "train_loss", training_loss, epoch=epoch)
                print(f"Epoch {epoch}/{config.epochs}: loss={training_loss:.6f}")

            test_loss, test_accuracy = _evaluate(torch, model, test_loader, loss_function)
            evaluation = {"accuracy": test_accuracy, "loss": test_loss}
            if comet_run is not None:
                _mirror_to_comet("logging test loss", comet_run.experiment.log_metric, "test_loss", test_loss)
                _mirror_to_comet("logging test accuracy", comet_run.experiment.log_metric, "test_accuracy", test_accuracy)

            checkpoint = {
                "configuration": configuration,
                "evaluation": evaluation,
                "model_state_dict": model.state_dict(),
                "optimizer_state_dict": optimizer.state_dict(),
                "training_metrics": training_metrics,
            }
            checkpoint_path = artifact_directory / "checkpoint.pt"
            torch.save(checkpoint, checkpoint_path)
            if comet_run is not None:
                _mirror_to_comet("logging the checkpoint", comet_run.log_model, comet_run.experiment, checkpoint, model_name="cdt-mnist")

            dataset_root = config.data_directory / "MNIST"
            run_record = {
                "artifacts": {
                    "checkpoint": artifact_record(checkpoint_path, artifact_directory),
                    "configuration": artifact_record(configuration_path, artifact_directory),
                },
                "configuration": configuration,
                "dataset_files": _dataset_manifest(dataset_root),
                "evaluation": evaluation,
                "training_metrics": training_metrics,
            }
            write_json(artifact_directory / "run.json", run_record)
        finally:
            if comet_run is not None:
                _mirror_to_comet("ending the experiment", comet_run.experiment.end)

    print(f"Test loss={test_loss:.6f}, accuracy={test_accuracy:.4%}")
    print(f"Canonical local artifacts: {config.output_directory}")


def main(argv: Sequence[str] | None = None) -> int:
    """Run the MNIST experiment from an installed uv entry point."""
    try:
        config = _config_from_args(_parse_args(sys.argv[1:] if argv is None else argv))
        _run_experiment(config)
    except ModuleNotFoundError as error:
        print(
            f"Missing experiment dependency {error.name!r}; run `just python-sync-experiments`, then retry with `uv run --no-sync cdt-mnist-experiment`.",
            file=sys.stderr,
        )
        return 2
    except (ExperimentConfigurationError, OutputDirectoryExistsError) as error:
        print(str(error), file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
