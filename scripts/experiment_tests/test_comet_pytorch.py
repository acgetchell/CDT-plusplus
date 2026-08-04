"""Offline integration test for Comet's current PyTorch API."""

import json
import unittest
from pathlib import Path, PurePosixPath
from tempfile import TemporaryDirectory
from zipfile import ZipFile

from scripts.mnist_experiment import _build_model, _RunConfig, _start_comet


class CometPyTorchIntegrationTests(unittest.TestCase):
    """Exercise start, watch, explicit logging, and checkpoint logging offline."""

    def test_offline_comet_run_records_a_pytorch_checkpoint(self) -> None:
        """The integration needs neither credentials nor hosted services."""
        with TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            config = _RunConfig(
                batch_size=2,
                comet_mode="offline",
                data_directory=root / "data",
                download=False,
                epochs=1,
                learning_rate=0.001,
                output_directory=root / "run",
                seed=0,
            )
            comet_run = _start_comet(config, config.output_directory)
            try:
                import torch  # noqa: PLC0415

                torch.manual_seed(config.seed)
                model = _build_model(torch)
                comet_run.watch(model)
                inputs = torch.zeros((2, 1, 28, 28))
                labels = torch.tensor([0, 1])
                loss = torch.nn.CrossEntropyLoss()(model(inputs), labels)
                loss.backward()
                comet_run.experiment.log_parameters({"seed": config.seed})
                comet_run.experiment.log_metric("loss", float(loss.item()), epoch=1)
                checkpoint = {"loss": float(loss.item()), "model_state_dict": model.state_dict()}
                comet_run.log_model(comet_run.experiment, checkpoint, model_name="cdt-mnist-smoke")
            finally:
                comet_run.experiment.end()

            archives = list((config.output_directory / "comet").glob("*.zip"))
            self.assertEqual(len(archives), 1)
            with ZipFile(archives[0]) as archive:
                records = [json.loads(line) for line in archive.read("messages.json").splitlines()]

            metrics = [record["payload"].get("metric", {}) for record in records if record["type"] == "metric_msg"]
            parameters = [record["payload"].get("param", {}) for record in records if record["type"] == "parameter_msg"]
            uploads = [record["payload"] for record in records if record["type"] == "file_upload"]
            self.assertTrue(any(metric.get("metricName") == "loss" and metric.get("epoch") == 1 for metric in metrics))
            self.assertTrue(any(parameter.get("paramName") == "seed" and parameter.get("paramValue") == 0 for parameter in parameters))
            self.assertTrue(any(record["type"] == "graph" for record in records))
            self.assertTrue(any(upload["upload_type"] == "histogram3d" for upload in uploads))
            self.assertTrue(
                any(
                    upload["upload_type"] == "model-element"
                    and upload["additional_params"].get("groupingName") == "cdt-mnist-smoke"
                    and PurePosixPath(str(upload["additional_params"].get("fileName", "")).replace("\\", "/")).parent == PurePosixPath("model-data")
                    and PurePosixPath(str(upload["additional_params"].get("fileName", "")).replace("\\", "/")).name == "comet-torch-model.pth"
                    for upload in uploads
                )
            )


if __name__ == "__main__":
    unittest.main()
