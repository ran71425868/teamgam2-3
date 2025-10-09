#include "Collision.h"

//球と球の交差判定
bool Collision::IntersectSphereVsSphere(
	const DirectX::XMFLOAT3& positionA,
	float radiusA,
	const DirectX::XMFLOAT3& positionB,
	float radiusB,
	DirectX::XMFLOAT3& outPositionB) 
{
	//A→Bの単位ベクトルを算出
	DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&positionA);//XMFLOAT3をXMVECTORに変換
	DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&positionB);
	DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB,PositionA);//pB-pA
	DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);//三平方の定理でVecの長さを計算
	float lengthSq;
	DirectX::XMStoreFloat(&lengthSq, LengthSq);

	//距離判定
	float range = radiusA + radiusB;
	if (lengthSq > range * range) 
	{
		return false;
	}

	//AがBを押し出す
	Vec = DirectX::XMVector3Normalize(Vec);//pBへの方向にベクトルを単位ベクトルに直す
	Vec = DirectX::XMVectorScale(Vec, range);//単位ベクトル方向に半径分引き伸ばす
	PositionB = DirectX::XMVectorAdd(PositionA, Vec);//PositionA+半径分引き伸ばした位置を押し出し位置に設定して
	DirectX::XMStoreFloat3(&outPositionB, PositionB);//XMFLOAT3に戻す

	return true;
}

//円柱と円柱の交差判定
bool Collision::IntersectCylinderVsCylinder(const DirectX::XMFLOAT3& positionA, 
	float radiusA, 
	float heightA,
	const DirectX::XMFLOAT3& positionB, 
	float radiusB, 
	float heightB,
	DirectX::XMFLOAT3& outPositionB)
{
	//Aの足元がBの頭より上なら当たってない
	if (positionA.y > positionB.y + heightB)
	{
		return false;
	}

	//Aの頭がBの足元より下なら当たっていない
	if (positionA.y + heightA < positionB.y)
	{
		return false;
	}

	//XZ平面での範囲チェック
	//x成分を引き算
	float vx = positionB.x - positionA.x;
	//z成分を引き算
	float vz = positionB.z - positionA.z;
	//半径の合計を計算
	float range = radiusA + radiusB;
	//AからBへのベクトルを求める
	float distXZ = sqrtf(vx * vx + vz * vz);
	if (distXZ > range)
	{
		return false;
	}

	//単位ベクトル化
	vx /= distXZ;
	vz /= distXZ;


	//AがBを押し出す
	outPositionB.x = positionA.x + (range * vx);
	outPositionB.y = positionB.y;
	outPositionB.z = positionA.z + (range * vz);

	return true;
}

//球と円柱の交差判定
bool Collision::IntersectSphereVsCylinder(
	const DirectX::XMFLOAT3& spherePosition, 
	float sphereRadius, 
	const DirectX::XMFLOAT3& cylinderPosition, 
	float cylinderRadius, 
	float cylinderHeight, 
	DirectX::XMFLOAT3& outCylinderPosition)
{


	//球の足元（位置-半径）が円柱の頭より上なら当たってない
	if (spherePosition.y-sphereRadius > cylinderPosition.y + cylinderHeight)
	{
		return false;
	}

	//球の頭（位置+半径）が円柱の足元より下なら当たっていない
	if (spherePosition.y + sphereRadius < cylinderPosition.y)
	{
		return false;
	}

	//XZ平面での範囲チェック
	//x成分を引き算
	float vx = cylinderPosition.x - spherePosition.x;
	//z成分を引き算
	float vz = cylinderPosition.z - spherePosition.z;
	//半径の合計を計算
	float range = sphereRadius + cylinderRadius;
	//AからBへのベクトルを求める
	float distXZ = sqrtf(vx * vx + vz * vz);
	if (distXZ > range)
	{
		return false;
	}

	//単位ベクトル化
	vx /= distXZ;
	vz /= distXZ;


	//AがBを押し出す
	outCylinderPosition.x = spherePosition.x + (range * vx);
	outCylinderPosition.y = cylinderPosition.y;
	outCylinderPosition.z = spherePosition.z + (range * vz);

	return true;
}
