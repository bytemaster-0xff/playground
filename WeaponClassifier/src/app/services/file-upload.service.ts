import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable } from 'rxjs/observable';
import { Result } from '../models/PredictionResults';

@Injectable()
export class FileUploadService {

    constructor(private httpClient: HttpClient) { }

    handleError(e: any) {

    }

    postFile(fileToUpload: File): Observable<Result> {
        const endpoint = 'https://southcentralus.api.cognitive.microsoft.com/customvision/v1.1/Prediction/3420d207-817e-4c2b-8b22-9e6929f405d8/image';
        const formData: FormData = new FormData();
        formData.append('fileKey', fileToUpload, fileToUpload.name);

        const headers = {
            'ContentType': 'application/octet-stream',
            'Prediction-Key': '6c10f03ea1c94db182ddcefce20ab3b0'
        };

        return this.httpClient.post<Result>(endpoint, formData, { headers: headers });
    }

}
