#include "Pch.h"
#include "Camera.h"
#include "Globals.h"
#include "ConfigUtilities.h"

void Camera::UpdateCamera(VMMDLL_SCATTER_HANDLE handle)
{
    uintptr_t systemBase = EnvironmentInstance->GetpSystem() + CameraBase;
    TargetProcess.AddScatterReadRequest(handle, systemBase + CameraPosOffset, &Position, sizeof(Vector3));
    TargetProcess.AddScatterReadRequest(handle, systemBase + ViewMatrixOffset, &RenderMatrix, sizeof(ViewMatrix));
    TargetProcess.AddScatterReadRequest(handle, systemBase + ProjectionMatrixOffset, &ProjectionMatrix, sizeof(ViewMatrix));
}

Vector2 Camera::WorldToScreen(Vector3 pos, bool clamp)
{
    if (Position == Vector3::Zero())
        return Vector2::Zero();

    // Precompute matrix operations
    Vector4 transformed = {
        pos.x * RenderMatrix.matrix[0][0] + pos.y * RenderMatrix.matrix[1][0] + pos.z * RenderMatrix.matrix[2][0] + RenderMatrix.matrix[3][0],
        pos.x * RenderMatrix.matrix[0][1] + pos.y * RenderMatrix.matrix[1][1] + pos.z * RenderMatrix.matrix[2][1] + RenderMatrix.matrix[3][1],
        pos.x * RenderMatrix.matrix[0][2] + pos.y * RenderMatrix.matrix[1][2] + pos.z * RenderMatrix.matrix[2][2] + RenderMatrix.matrix[3][2],
        pos.x * RenderMatrix.matrix[0][3] + pos.y * RenderMatrix.matrix[1][3] + pos.z * RenderMatrix.matrix[2][3] + RenderMatrix.matrix[3][3]
    };

    // Check visibility
    if (transformed.z >= 0.0f)
        return Vector2::Zero();

    // Projection
    Vector4 projected = {
        transformed.x * ProjectionMatrix.matrix[0][0] + transformed.y * ProjectionMatrix.matrix[1][0] + transformed.z * ProjectionMatrix.matrix[2][0] + transformed.w * ProjectionMatrix.matrix[3][0],
        transformed.x * ProjectionMatrix.matrix[0][1] + transformed.y * ProjectionMatrix.matrix[1][1] + transformed.z * ProjectionMatrix.matrix[2][1] + transformed.w * ProjectionMatrix.matrix[3][1],
        transformed.x * ProjectionMatrix.matrix[0][2] + transformed.y * ProjectionMatrix.matrix[1][2] + transformed.z * ProjectionMatrix.matrix[2][2] + transformed.w * ProjectionMatrix.matrix[3][2],
        transformed.x * ProjectionMatrix.matrix[0][3] + transformed.y * ProjectionMatrix.matrix[1][3] + transformed.z * ProjectionMatrix.matrix[2][3] + transformed.w * ProjectionMatrix.matrix[3][3]
    };

    if (projected.w == 0.0f)
        return Vector2::Zero();

    // Divide by w
    projected.x /= projected.w;
    projected.y /= projected.w;

    if (clamp && (abs(projected.x) > 1.0f || abs(projected.y) > 1.0f))
        return Vector2::Zero();

    // Get screen coordinates
    int width = Configs.Overlay.OverrideResolution ? Configs.Overlay.Width : GetSystemMetrics(SM_CXSCREEN);
    int height = Configs.Overlay.OverrideResolution ? Configs.Overlay.Height : GetSystemMetrics(SM_CYSCREEN);

    return Vector2{
        (1.0f + projected.x) * width * 0.5f,
        (1.0f - projected.y) * height * 0.5f
    };
}
