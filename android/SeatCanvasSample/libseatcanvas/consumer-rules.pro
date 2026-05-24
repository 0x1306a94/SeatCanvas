# === 1. Public API ===
# Keep all public classes in the library and their public/protected members.
-keep class com.libseatcanvas.** {
    public protected <init>(...);
    public protected *;
}

# === 2. Native methods (JNI) ===
# Keep all native methods and their class names intact.
-keepclasseswithmembernames,includedescriptorclasses class * {
    native <methods>;
}

# === 3. JNI-accessed internals of SeatCanvasView ===
# These are private members not covered by rules 1 & 2, but accessed by C++ via GetFieldID / GetMethodID.
-keepclassmembers class com.libseatcanvas.SeatCanvasView {
    *** nativePtr;
    private void nativeOnDidLoadBaseMap();
    private void nativeOnDidUnloadBaseMap();
    private void nativeOnDidUpdateZoomLevelConfig(float, float, float, float, float, float, float);
    private void nativeOnDidTapZone(java.lang.String);
    private boolean nativeOnDidTapSeat(java.lang.String, java.lang.String);
    private void nativeOnViewportWillBeginDragging(float, float, float, float, float, float, float);
    private void nativeOnViewportDidScroll(float, float, float, float, float, float, float);
    private void nativeOnViewportDidEndDragging(float, float, float, float, float, float, float, boolean);
    private void nativeOnViewportDidEndDecelerating(float, float, float, float, float, float, float);
    private void nativeOnViewportWillBeginZooming(float, float, float, float, float, float, float);
    private void nativeOnViewportDidZoom(float, float, float, float, float, float, float);
    private void nativeOnViewportDidEndZooming(float, float, float, float, float, float, float);
    private void nativeOnViewportDidEndScrollingAnimation(float, float, float, float, float, float, float);
}
