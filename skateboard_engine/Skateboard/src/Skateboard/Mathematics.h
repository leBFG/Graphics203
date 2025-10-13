#pragma once
#define SKTBD_PI 3.141592654f

//#define GLM_FORCE_SSE2
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext.hpp"
#include "glm/gtx/matrix_decompose.hpp"

// TODO: Remove this! and use glm functions everywhere!
typedef glm::vec2 float2;
typedef glm::vec3 float3;
typedef glm::vec4 float4;
typedef glm::ivec2 int2;
typedef glm::ivec3 int3;
typedef glm::ivec4 int4;
typedef glm::uvec2 uint2;
typedef glm::uvec3 uint3;
typedef glm::uvec4 uint4;
typedef glm::vec4 vector;
typedef glm::mat4 matrix, float4x4;

typedef glm::mat<4, 4, glm::f32, glm::defaultp>	float4x3;

//struct BoundSphere
//{
//	float3 Center;
//	float Radius;
//};

//struct BoundBox
//{
//	float3 Center;
//	float3 Extents;
//};


struct Transform
{
	Transform():
		Translation(0.f, 0.f, 0.f),
		Rotation(1.f, 0.f, 0.f, 0.f),
		Scale(1.f, 1.f, 1.f)
	{
		
	};
	
	const glm::mat4x4 AsMatrix() const {
		return glm::translate(Translation) * glm::toMat4(Rotation) * glm::scale(Scale);
	}

	Transform operator*(const Transform& A) {
		Transform T = {};
		auto m = AsMatrix() * A.AsMatrix();
		T = m;
		return T;
	}
	// <summary>
	// Operator overload for 4x4 matrix decomposition. Extracts the translation, rotation and scale
	// properties encoded in the matrix.
	// </summary>
	__inline auto operator= (const glm::mat4x4& mat) -> Transform& {
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(mat, Scale, Rotation, Translation, skew, perspective);
		return *this;
	}

	float AsEulerX() const { return glm::eulerAngles(Rotation).x; }
	float AsEulerY() const { return glm::eulerAngles(Rotation).y; }
	float AsEulerZ() const { return glm::eulerAngles(Rotation).z; }

	glm::vec3 AsEuler() const { return glm::eulerAngles(Rotation); }

	void SetEulerX(float x) { SetEulerAngles(x, AsEulerY(), AsEulerZ()); }
	void SetEulerY(float y) { SetEulerAngles(AsEulerX(), y, AsEulerZ()); }
	void SetEulerZ(float z) { SetEulerAngles(AsEulerX(), AsEulerY(), z); }

	void SetEulerAngles(float x, float y, float z) 
	{ 
		Rotation = glm::toQuat(glm::yawPitchRoll(y, x, z));
	}
	// Add a rotation quat to current transform rotation
	void Rotate(const glm::quat& deltaRot)
	{
		Rotation *= deltaRot;
	};
	// Add a rotation from x,y,z to current transform rotation
	void Rotate(const glm::vec3& eulerDeltaRot)
	{
		Rotate(glm::quat(eulerDeltaRot));
	};

	glm::vec3 GetForwardVector() const {
		auto fvec = glm::normalize(Rotation * glm::vec3(0, 0, 1));
		return fvec;
	}

	glm::vec3 const GetRightVector() const {
		auto fvec = glm::normalize(Rotation * glm::vec3(1, 0, 0));
		return fvec;
	}

	void LookAt(glm::vec3 target) {
		float3 dir = glm::normalize(target - Translation);
		Rotation = glm::quatLookAtLH(dir, float3(0,1,0));	// Set the rotation, by giving it  a target direction and up vector
	}

	glm::vec3 const GetUpVector() const {
		glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);  // Set w to 1 for the following matrix multiplication
		up = Rotation * up;
		return up;
	}

	glm::vec3 Translation;
	glm::quat Rotation;
	glm::vec3 Scale;
};

/*
#ifdef SKTBD_PLATFORM_WINDOWS
#include <DirectXMath.h>
#include <DirectXCollision.h>
typedef DirectX::XMFLOAT2 float2;
typedef DirectX::XMFLOAT3 float3;
typedef DirectX::XMFLOAT4 float4;
typedef DirectX::XMINT2 int2;
typedef DirectX::XMINT3 int3;
typedef DirectX::XMINT4 int4;
typedef DirectX::XMUINT2 uint2;
typedef DirectX::XMUINT3 uint3;
typedef DirectX::XMUINT4 uint4;
typedef DirectX::XMVECTOR vector;
typedef DirectX::FXMVECTOR fvector;
typedef DirectX::XMMATRIX matrix, float4x4;
typedef DirectX::BoundingSphere BoundSphere;
typedef DirectX::BoundingBox BoundBox;

namespace Skateboard
{
	// -----------------------------------------------------------------------------
	// Angles
	// -----------------------------------------------------------------------------
	inline auto RadToDeg(float radians) -> decltype(DirectX::XMConvertToDegrees(radians)) {
		return DirectX::XMConvertToDegrees(radians);
	}
	inline auto DegToRad(float degrees) -> decltype(DirectX::XMConvertToRadians(degrees)) {
		return DirectX::XMConvertToRadians(degrees);
	}

	// -----------------------------------------------------------------------------
	// 4D Vectors
	// -----------------------------------------------------------------------------
	inline auto VectorSet(float xxxx) -> decltype(DirectX::XMVectorSet(xxxx, xxxx, xxxx, xxxx)) {
		return DirectX::XMVectorSet(xxxx, xxxx, xxxx, xxxx);
	}
	inline auto VectorSet(float x, float y, float z, float w) -> decltype(DirectX::XMVectorSet(x, y, z, w)) {
		return DirectX::XMVectorSet(x, y, z, w);
	}
	inline auto VectorAdd(vector v1, vector v2) -> decltype(DirectX::XMVectorAdd(v1, v2)) {
		return DirectX::XMVectorAdd(v1, v2);
	}
	inline auto VectorSub(vector v1, vector v2) -> decltype(DirectX::XMVectorSubtract(v1, v2)) {
		return DirectX::XMVectorSubtract(v1, v2);
	}
	inline auto VectorMul(vector v1, vector v2) -> decltype(DirectX::XMVectorMultiply(v1, v2)) {
		return DirectX::XMVectorMultiply(v1, v2);
	}
	inline auto VectorDiv(vector v1, vector v2) -> decltype(DirectX::XMVectorDivide(v1, v2)) {
		return DirectX::XMVectorDivide(v1, v2);
	}
	inline auto VectorGetX(vector v) -> decltype(DirectX::XMVectorGetX(v)) {
		return DirectX::XMVectorGetX(v);
	}
	inline auto VectorGetY(vector v) -> decltype(DirectX::XMVectorGetY(v)) {
		return DirectX::XMVectorGetY(v);
	}
	inline auto VectorGetZ(vector v) -> decltype(DirectX::XMVectorGetZ(v)) {
		return DirectX::XMVectorGetZ(v);
	}
	inline auto VectorReplicate(float x) ->decltype(DirectX::XMVectorReplicate(x)){
		return DirectX::XMVectorReplicate(x);
	}
	// -----------------------------------------------------------------------------
	// 3D Vectors
	// -----------------------------------------------------------------------------
	inline auto Vector3Load(const float3* pSource) -> decltype(DirectX::XMLoadFloat3(pSource)) {
		return DirectX::XMLoadFloat3(pSource);
	}
	inline auto Vector3Store(float3* pDest, vector source) -> decltype(DirectX::XMStoreFloat3(pDest, source)) {
		return DirectX::XMStoreFloat3(pDest, source);
	}
	inline auto Vector3AngleBetweenNormals(vector n1, vector n2) -> decltype(DirectX::XMVector3AngleBetweenNormals(n1, n2)) {
		return DirectX::XMVector3AngleBetweenNormals(n1, n2);
	}
	inline auto Vector3AngleBetweenVectors(vector v1, vector v2) -> decltype(DirectX::XMVector3AngleBetweenVectors(v1, v2)) {
		return DirectX::XMVector3AngleBetweenVectors(v1, v2);
	}
	inline auto Vector3Dot(vector v1, vector v2) -> decltype(DirectX::XMVector3Dot(v1, v2)) {
		return DirectX::XMVector3Dot(v1, v2);
	}
	inline auto Vector3Cross(vector v1, vector v2) -> decltype(DirectX::XMVector3Cross(v1, v2)) {
		return DirectX::XMVector3Cross(v1, v2);
	}
	inline auto Vector3Length(vector v) -> decltype(DirectX::XMVector3Length(v)) {
		return DirectX::XMVector3Length(v);
	}
	inline auto Vector3Normalise(vector v) -> decltype(DirectX::XMVector3Normalize(v)) {
		return DirectX::XMVector3Normalize(v);
	}
	inline auto Vector3Rotate(vector v, vector rotationQuaternion) -> decltype(DirectX::XMVector3Rotate(v, rotationQuaternion)) {
		return DirectX::XMVector3Rotate(v, rotationQuaternion);
	}
	inline auto Vector3Transform(vector v, float4x4 m) -> decltype(DirectX::XMVector3Transform(v, m)) {
		return DirectX::XMVector3Transform(v, m);
	}
	inline auto Vector3TransformCoord(vector v, float4x4 m) -> decltype(DirectX::XMVector3TransformCoord(v, m)) {
		return DirectX::XMVector3TransformCoord(v, m);
	}
	inline auto Vector3Dot(vector v, float4x4 m) -> decltype(DirectX::XMVector3TransformNormal(v, m)) {
		return DirectX::XMVector3TransformNormal(v, m);
	}

	// -----------------------------------------------------------------------------
	// Quaternions
	// -----------------------------------------------------------------------------
	inline auto QuaternionRotationAxis(vector axis, float angle) -> decltype(DirectX::XMQuaternionRotationAxis(axis, angle)) {
		return DirectX::XMQuaternionRotationAxis(axis, angle);
	}
	inline auto QuaternionRotationNormal(vector normalAxis, float angle) -> decltype(DirectX::XMQuaternionRotationNormal(normalAxis, angle)) {
		return DirectX::XMQuaternionRotationNormal(normalAxis, angle);
	}
	inline auto QuaternionRotationMatrix(float4x4 m) -> decltype(DirectX::XMQuaternionRotationMatrix(m)) {
		return DirectX::XMQuaternionRotationMatrix(m);
	}
	inline auto QuaternionPitchYawRoll(float pitch, float yaw, float roll) -> decltype(DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll)) {
		return DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
	}
	inline auto QuaternionPitchYawRollFromVector(vector vectorPitchYawRoll) -> decltype(DirectX::XMQuaternionRotationRollPitchYawFromVector(vectorPitchYawRoll)) {
		return DirectX::XMQuaternionRotationRollPitchYawFromVector(vectorPitchYawRoll);
	}
	inline auto QuaternionToAxisAngle(vector* pAxis, float* pAngle, vector quaternion) -> decltype(DirectX::XMQuaternionToAxisAngle(pAxis, pAngle, quaternion)) {
		return DirectX::XMQuaternionToAxisAngle(pAxis, pAngle, quaternion);
	}

	// -----------------------------------------------------------------------------
	// Matrices
	// -----------------------------------------------------------------------------
	inline auto MatrixIdentity() -> decltype(DirectX::XMMatrixIdentity()) {
		return DirectX::XMMatrixIdentity();
	}
	inline auto MatrixTranspose(float4x4 m) -> decltype(DirectX::XMMatrixTranspose(m)) {
		return DirectX::XMMatrixTranspose(m);
	}
	inline auto MatrixInverse(vector* pDeterminant, float4x4 m) -> decltype(DirectX::XMMatrixInverse(pDeterminant, m)) {
		return DirectX::XMMatrixInverse(pDeterminant, m);
	}
	inline auto MatrixTranslation(float offsetX, float offsetY, float offsetZ) -> decltype(DirectX::XMMatrixTranslation(offsetX, offsetY, offsetZ)) {
		return DirectX::XMMatrixTranslation(offsetX, offsetY, offsetZ);
	}
	inline auto MatrixTranslationFromVector(vector translationVector) -> decltype(DirectX::XMMatrixTranslationFromVector(translationVector)) {
		return DirectX::XMMatrixTranslationFromVector(translationVector);
	}
	inline auto MatrixRotationAxis(vector axis, float angle) -> decltype(DirectX::XMMatrixRotationAxis(axis, angle)) {
		return DirectX::XMMatrixRotationAxis(axis, angle);
	}
	inline auto MatrixRotationNormal(vector normalAxis, float angle) -> decltype(DirectX::XMMatrixRotationNormal(normalAxis, angle)) {
		return DirectX::XMMatrixRotationNormal(normalAxis, angle);
	}
	inline auto MatrixRotationQuaternion(vector rotationQuaternion) -> decltype(DirectX::XMMatrixRotationQuaternion(rotationQuaternion)) {
		return DirectX::XMMatrixRotationQuaternion(rotationQuaternion);
	}
	inline auto MatrixRotationPitchYawRoll(float pitch, float yaw, float roll) -> decltype(DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll)) {
		return DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
	}
	inline auto MatrixRotationPitchYawRollFromVector(vector angles) -> decltype(DirectX::XMMatrixRotationRollPitchYawFromVector(angles)) {
		return DirectX::XMMatrixRotationRollPitchYawFromVector(angles);
	}
	inline auto MatrixRotationX(float angle) -> decltype(DirectX::XMMatrixRotationX(angle)) {
		return DirectX::XMMatrixRotationX(angle);
	}
	inline auto MatrixRotationY(float angle) -> decltype(DirectX::XMMatrixRotationY(angle)) {
		return DirectX::XMMatrixRotationY(angle);
	}
	inline auto MatrixRotationZ(float angle) -> decltype(DirectX::XMMatrixRotationZ(angle)) {
		return DirectX::XMMatrixRotationZ(angle);
	}
	inline auto MatrixScaling(float scaleX, float scaleY, float scaleZ) -> decltype(DirectX::XMMatrixScaling(scaleX, scaleY, scaleZ)) {
		return DirectX::XMMatrixScaling(scaleX, scaleY, scaleZ);
	}
	inline auto MatrixScalingFromVector(vector scaleVector) -> decltype(DirectX::XMMatrixScalingFromVector(scaleVector)) {
		return DirectX::XMMatrixScalingFromVector(scaleVector);
	}
	inline auto MatrixLookAt(vector eyePosition, vector focusPoint, vector upDirection) -> decltype(DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection)) {
		return DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
	}
	inline auto MatrixPerspectiveFov(float fov, float aspectRatio, float nearPlane, float farPlane) -> decltype(DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane)) {
		return DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);
	}
	inline auto MatrixOrthographic(float viewWidth, float viewHeight, float nearPlane, float farPlane) -> decltype(DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, nearPlane, farPlane)) {
		return DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, nearPlane, farPlane);
	}
	
	inline auto MatrixDecompose(vector* out_Scale, vector* out_Rotation, vector* out_Translation, float4x4 in_Matrix) -> decltype(DirectX::XMMatrixDecompose(out_Scale, out_Rotation, out_Translation, in_Matrix)) {
		bool returnval = DirectX::XMMatrixDecompose(out_Scale, out_Rotation, out_Translation, in_Matrix);

		// Quaternion to Euler
		// TODO: Broken
		float3 rotationEuler;
		float3 mRow1, mRow2, mRow3;
		Vector3Store(&mRow1, in_Matrix.r[0]);
		Vector3Store(&mRow2, in_Matrix.r[1]);
		Vector3Store(&mRow3, in_Matrix.r[2]);
		rotationEuler.y = asinf(-mRow1.z);
		if (cosf(rotationEuler.y))
		{
			rotationEuler.x = atan2f(mRow2.z, mRow3.z);
			rotationEuler.z = atan2f(mRow1.y, mRow1.x);
		}
		else
		{
			rotationEuler.x = atan2f(-mRow2.x, mRow1.y);
			rotationEuler.z = 0.f;
		}
		rotationEuler = { RadToDeg(rotationEuler.x), RadToDeg(rotationEuler.y), RadToDeg(rotationEuler.z) };
		*out_Rotation = Vector3Load(&rotationEuler);

		return returnval;
	}
}
#endif

#ifdef SKTBD_PLATFORM_PLAYSTATION
#include <vectormath.h>
using namespace sce::Vectormath;
struct float2 { float x; float y;								float2() = default; float2(float x, float y) : x(x), y(y) {} };
struct float3 { float x; float y; float z;						float3() = default; float3(float x, float y, float z) : x(x), y(y), z(z) {} };
struct float4 { float x; float y; float z; float w;				float4() = default; float4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {} };
struct int2 { int32_t x; int32_t y;								int2()	 = default; int2(int32_t x, int32_t y) : x(x), y(y) {} };
struct int3 { int32_t x; int32_t y; int32_t z;					int3()	 = default; int3(int32_t x, int32_t y, int32_t z) : x(x), y(y), z(z) {} };
struct int4 { int32_t x; int32_t y; int32_t z; int32_t w;		int4()	 = default; int4(int32_t x, int32_t y, int32_t z, int32_t w) : x(x), y(y), z(z), w(w) {} };
struct uint2 { uint32_t x; uint32_t y;							uint2()  = default; uint2(uint32_t x, uint32_t y) : x(x), y(y) {} };
struct uint3 { uint32_t x; uint32_t y; uint32_t z;				uint3()  = default; uint3(uint32_t x, uint32_t y, uint32_t z) : x(x), y(y), z(z) {} };
struct uint4 { uint32_t x; uint32_t y; uint32_t z; uint32_t w;	uint4()  = default; uint4(uint32_t x, uint32_t y, uint32_t z, uint32_t w) : x(x), y(y), z(z), w(w) {} };
//typedef Scalar::Aos::Vector3 float3;
typedef Simd::Aos::Vector4 vector;
typedef Simd::Aos::Matrix4 matrix; 
typedef Scalar::Aos::Matrix4 float4x4;
typedef Scalar::Aos::Matrix4_arg float4x4_arg;
typedef Simd::Aos::Quat quaternion;

// -----------------------------------------------------------------------------
	// Angles
	// -----------------------------------------------------------------------------
inline auto RadToDeg(float radians) -> decltype(DirectX::XMConvertToDegrees(radians)) {
	return DirectX::XMConvertToDegrees(radians);
}
inline auto DegToRad(float degrees) -> decltype(DirectX::XMConvertToRadians(degrees)) {
	return DirectX::XMConvertToRadians(degrees);
}

// -----------------------------------------------------------------------------
// 4D Vectors
// -----------------------------------------------------------------------------
inline auto VectorSet(float xxxx) -> decltype(DirectX::XMVectorSet(xxxx, xxxx, xxxx, xxxx)) {
	return DirectX::XMVectorSet(xxxx, xxxx, xxxx, xxxx);
}
inline auto VectorSet(float x, float y, float z, float w) -> decltype(DirectX::XMVectorSet(x, y, z, w)) {
	return DirectX::XMVectorSet(x, y, z, w);
}
inline auto VectorAdd(vector v1, vector v2) -> decltype(DirectX::XMVectorAdd(v1, v2)) {
	return DirectX::XMVectorAdd(v1, v2);
}
inline auto VectorSub(vector v1, vector v2) -> decltype(DirectX::XMVectorSubtract(v1, v2)) {
	return DirectX::XMVectorSubtract(v1, v2);
}
inline auto VectorMul(vector v1, vector v2) -> decltype(DirectX::XMVectorMultiply(v1, v2)) {
	return DirectX::XMVectorMultiply(v1, v2);
}
inline auto VectorDiv(vector v1, vector v2) -> decltype(DirectX::XMVectorDivide(v1, v2)) {
	return DirectX::XMVectorDivide(v1, v2);
}
inline auto VectorGetX(vector v) -> decltype(DirectX::XMVectorGetX(v)) {
	return DirectX::XMVectorGetX(v);
}
inline auto VectorGetY(vector v) -> decltype(DirectX::XMVectorGetY(v)) {
	return DirectX::XMVectorGetY(v);
}
inline auto VectorGetZ(vector v) -> decltype(DirectX::XMVectorGetZ(v)) {
	return DirectX::XMVectorGetZ(v);
}

// -----------------------------------------------------------------------------
// 3D Vectors
// -----------------------------------------------------------------------------
inline auto Vector3Load(const float3* pSource) -> decltype(DirectX::XMLoadFloat3(pSource)) {
	return DirectX::XMLoadFloat3(pSource);
}
inline auto Vector3Store(float3* pDest, vector source) -> decltype(DirectX::XMStoreFloat3(pDest, source)) {
	return DirectX::XMStoreFloat3(pDest, source);
}
inline auto Vector3AngleBetweenNormals(vector n1, vector n2) -> decltype(DirectX::XMVector3AngleBetweenNormals(n1, n2)) {
	return DirectX::XMVector3AngleBetweenNormals(n1, n2);
}
inline auto Vector3AngleBetweenVectors(vector v1, vector v2) -> decltype(DirectX::XMVector3AngleBetweenVectors(v1, v2)) {
	return DirectX::XMVector3AngleBetweenVectors(v1, v2);
}
inline auto Vector3Dot(vector v1, vector v2) -> decltype(DirectX::XMVector3Dot(v1, v2)) {
	return DirectX::XMVector3Dot(v1, v2);
}
inline auto Vector3Cross(vector v1, vector v2) -> decltype(DirectX::XMVector3Cross(v1, v2)) {
	return DirectX::XMVector3Cross(v1, v2);
}
inline auto Vector3Length(vector v) -> decltype(DirectX::XMVector3Length(v)) {
	return DirectX::XMVector3Length(v);
}
inline auto Vector3Normalise(vector v) -> decltype(DirectX::XMVector3Normalize(v)) {
	return DirectX::XMVector3Normalize(v);
}
inline auto Vector3Rotate(vector v, vector rotationQuaternion) -> decltype(DirectX::XMVector3Rotate(v, rotationQuaternion)) {
	return DirectX::XMVector3Rotate(v, rotationQuaternion);
}
inline auto Vector3Transform(vector v, float4x4 m) -> decltype(DirectX::XMVector3Transform(v, m)) {
	return DirectX::XMVector3Transform(v, m);
}
inline auto Vector3TransformCoord(vector v, float4x4 m) -> decltype(DirectX::XMVector3TransformCoord(v, m)) {
	return DirectX::XMVector3TransformCoord(v, m);
}
inline auto Vector3Dot(vector v, float4x4 m) -> decltype(DirectX::XMVector3TransformNormal(v, m)) {
	return DirectX::XMVector3TransformNormal(v, m);
}

// -----------------------------------------------------------------------------
// Quaternions
// -----------------------------------------------------------------------------
inline auto QuaternionRotationAxis(vector axis, float angle) -> decltype(Simd::Aos::Quat::rotation(angle, axis.getXYZ())) {
	return Simd::Aos::Quat::rotation(angle, axis.getXYZ());
}
inline auto QuaternionRotationNormal(vector normalAxis, float angle) -> decltype(Simd::Aos::Quat::rotation(angle, normalAxis.getXYZ())) {
	return Simd::Aos::Quat::rotation(angle, normalAxis.getXYZ());
}
inline auto QuaternionRotationMatrix(float4x4 m) -> decltype(Simd::Aos::Quat::rotation(m)) {
	return Simd::Aos::Quat::rotation(m);
}
inline auto QuaternionPitchYawRoll(float pitch, float yaw, float roll) -> decltype(Simd::Aos::Quat(pitch, yaw, roll)) {
	return Simd::Aos::Quat::rotation(pitch, yaw, roll);
}
inline auto QuaternionPitchYawRollFromVector(vector vectorPitchYawRoll) -> decltype(Simd::Aos::Quat(vectorPitchYawRoll)) {
	return Simd::Aos::Quat(vectorPitchYawRoll);
}
inline auto QuaternionToAxisAngle(vector* pAxis, float* pAngle, vector quaternion) -> decltype(Simd::Aos::Quat(pAxis, pAngle, quaternion)) {
	return Simd::Aos::Quat(pAxis, pAngle, quaternion);
}

// -----------------------------------------------------------------------------
// Matrices
// -----------------------------------------------------------------------------
inline auto MatrixIdentity() -> decltype(Simd::Aos::Matrix4::identity()) {
	return Simd::Aos::Matrix4::identity();
}
inline auto MatrixTranspose(float4x4 m) -> decltype(Simd::Aos::transpose((Simd::Aos::Matrix4)m)) {
	return Simd::Aos::transpose((Simd::Aos::Matrix4)m);
}
inline auto MatrixInverse(vector* pDeterminant, float4x4_arg m) -> decltype(Simd::Aos::inverse((Simd::Aos::Matrix4)m)) {
	return Simd::Aos::inverse((Simd::Aos::Matrix4)m);
}
inline auto MatrixTranslation(float offsetX, float offsetY, float offsetZ) -> decltype(Simd::Aos::Matrix4::translation(vector(offsetX, offsetY, offsetZ, 0.0).getXYZ())) {
	return Simd::Aos::Matrix4::translation(vector(offsetX, offsetY, offsetZ, 0.0f).getXYZ());
}
inline auto MatrixTranslationFromVector(vector translationVector) -> decltype(Simd::Aos::Matrix4::translation(translationVector.getXYZ())) {
	return Simd::Aos::Matrix4::translation(translationVector.getXYZ());
}
inline auto MatrixRotationAxis(vector axis, float angle) -> decltype(Simd::Aos::Matrix4::rotation(angle, axis.getXYZ())) {
	return Simd::Aos::Matrix4::rotation(angle, axis.getXYZ());
}
inline auto MatrixRotationNormal(vector normalAxis, float angle) -> decltype(Simd::Aos::Matrix4::rotation(angle, normalAxis.getXYZ())) {
	return Simd::Aos::Matrix4::rotation(angle, normalAxis.getXYZ());
}
inline auto MatrixRotationQuaternion(quaternion rotationQuaternion) -> decltype(Simd::Aos::Matrix4::rotation(rotationQuaternion)) {
	return Simd::Aos::Matrix4::rotation(rotationQuaternion);
}
inline auto MatrixRotationPitchYawRoll(float pitch, float yaw, float roll) -> decltype(Simd::Aos::Matrix4::rotationX(pitch)* Simd::Aos::Matrix4::rotationY(yaw)* Simd::Aos::Matrix4::rotationZ(roll)) {
	return Simd::Aos::Matrix4::rotationX(pitch) * Simd::Aos::Matrix4::rotationY(yaw) * Simd::Aos::Matrix4::rotationZ(roll);
}
inline auto MatrixRotationPitchYawRollFromVector(vector angles) -> decltype(Simd::Aos::Matrix4::rotationX(angles.getX())* Simd::Aos::Matrix4::rotationY(angles.getY()) * Simd::Aos::Matrix4::rotationZ(angles.getZ())) {
	return Simd::Aos::Matrix4::rotationX(angles.getX()) * Simd::Aos::Matrix4::rotationY(angles.getY()) * Simd::Aos::Matrix4::rotationZ(angles.getZ());
}
inline auto MatrixRotationX(float angle) -> decltype(Simd::Aos::Matrix4::rotationX(angle)) {
	return Simd::Aos::Matrix4::rotationX(angle);
}
inline auto MatrixRotationY(float angle) -> decltype(Simd::Aos::Matrix4::rotationY(angle)) {
	return Simd::Aos::Matrix4::rotationY(angle);
}
inline auto MatrixRotationZ(float angle) -> decltype(Simd::Aos::Matrix4::rotationZ(angle)) {
	return Simd::Aos::Matrix4::rotationZ(angle);
}
inline auto MatrixScaling(float scaleX, float scaleY, float scaleZ) -> decltype(Simd::Aos::Matrix4::scale(vector(scaleX, scaleY, scaleZ, 0.0f).getXYZ())) {
	return Simd::Aos::Matrix4::scale(vector(scaleX, scaleY, scaleZ, 0.0f).getXYZ());
}
inline auto MatrixScalingFromVector(vector scaleVector) -> decltype(Simd::Aos::Matrix4::scale(scaleVector.getXYZ())) {
	return Simd::Aos::Matrix4::scale(scaleVector.getXYZ());
}
inline auto MatrixLookAt(vector eyePosition, vector focusPoint, vector upDirection) -> decltype(Simd::Aos::Matrix4::lookAt(Simd::Aos::Point3(eyePosition), Simd::Aos::Point3(focusPoint), upDirection.getXYZ())) {
	return Simd::Aos::Matrix4::lookAt(Simd::Aos::Point3(eyePosition), Simd::Aos::Point3(focusPoint), upDirection.getXYZ());
}
inline auto MatrixPerspectiveFov(float fov, float aspectRatio, float nearPlane, float farPlane) -> decltype(Simd::Aos::Matrix4::perspective(fov, aspectRatio, nearPlane, farPlane)) {
	return Simd::Aos::Matrix4::perspective(fov, aspectRatio, nearPlane, farPlane);
}
inline auto MatrixOrthographic(float viewWidth, float viewHeight, float nearPlane, float farPlane) -> decltype(Simd::Aos::Matrix4::orthographic(- viewWidth/2.f, viewWidth / 2.f, -viewHeight/2.f, viewHeight/2.f,  nearPlane, farPlane)) {
	return Simd::Aos::Matrix4::orthographic(-viewWidth / 2.f, viewWidth / 2.f, -viewHeight / 2.f, viewHeight / 2.f, nearPlane, farPlane);
}
inline auto MatrixDecompose(vector* out_Scale, vector* out_Rotation, vector* out_Translation, float4x4 in_Matrix) -> bool {
	return true;
}

#endif
*/