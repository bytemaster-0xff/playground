import { AfterViewInit, Component, ElementRef, ViewChild } from '@angular/core';
import { fromEvent, combineLatest, takeUntil, concatMap, tap } from "rxjs";

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.sass']
})
export class AppComponent implements AfterViewInit{
  title = 'room-builder';
  cx: any;

  @ViewChild("roomCanvas", { static: false }) myCanvas: ElementRef | undefined;

  ngAfterViewInit(): void {
    const mouseDown$ = fromEvent(this.myCanvas!.nativeElement, "mousedown");
    const mouseMove$ = fromEvent(this.myCanvas!.nativeElement, "mousemove");
    const mouseUp$ = fromEvent(this.myCanvas!.nativeElement, "mouseup");

    mouseDown$.pipe(concatMap(down => mouseMove$.pipe(takeUntil(mouseUp$))));

    const mouseDraw$ = mouseDown$.pipe(
      tap((e: MouseEvent) => {
        this.cx.moveTo(e.offsetX, e.offsetY);
      }),
      concatMap(() => mouseMove$.pipe(takeUntil(mouseUp$)))
    );

    mouseDraw$.subscribe((e: MouseEvent) => this.draw(e.offsetX, e.offsetY));
    }

}
