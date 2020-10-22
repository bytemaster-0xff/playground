import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';

import { HttpClientModule } from '@angular/common/http';
import { AppComponent } from './app.component';
import { WeaponUploadComponent } from './components/weapon-upload/weapon-upload.component';
import { FileUploadService } from './services/file-upload.service';


@NgModule({
  declarations: [
    AppComponent,
    WeaponUploadComponent
  ],
  imports: [
    BrowserModule,
    HttpClientModule
  ],
  providers: [
    FileUploadService
  ],
  bootstrap: [AppComponent]
})
export class AppModule { }
