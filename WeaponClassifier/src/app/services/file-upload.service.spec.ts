/* tslint:disable:no-unused-variable */

import { TestBed, async, inject } from '@angular/core/testing';
import { FileUploadService } from './file-upload.service';

describe('Service: FileUpload', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [FileUploadService]
    });
  });

  it('should ...', inject([FileUploadService], (service: FileUploadService) => {
    expect(service).toBeTruthy();
  }));
});