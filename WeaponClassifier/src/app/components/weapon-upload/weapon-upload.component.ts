import { Component, OnInit } from '@angular/core';
import { FileUploadService } from '../../services/file-upload.service';
import { Prediction } from '../../models/PredictionResults';

@Component({
  selector: 'app-weapon-upload',
  templateUrl: './weapon-upload.component.html',
  styleUrls: ['./weapon-upload.component.css']
})
export class WeaponUploadComponent implements OnInit {

  fileToUpload: File = null;
  busy = false;
  showUploadButton = false;

  results: Prediction[];

  constructor(private fileUploadService: FileUploadService) { }
    ngOnInit() {

    }

  uploadFileToActivity() {
    this.results = null;
    this.busy = true;

    this.fileUploadService.postFile(this.fileToUpload)
    .subscribe(data => {
      this.busy = false;
      this.showUploadButton = false;
      this.results = data.Predictions;
      for (const prediction of data.Predictions) {
        console.log(prediction);
      }
      console.log(data);
      }, error => {
        console.log(error);
      });
  }

  handleFileInput(files: FileList) {
    this.fileToUpload = files.item(0);
    this.showUploadButton = true;
  }
}
