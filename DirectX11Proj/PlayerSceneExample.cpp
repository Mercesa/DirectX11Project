#include "PlayerSceneExample.h"


#include <iostream>

#include "ResourceManager.h"
#include <windows.h>


PlayerSceneExample::PlayerSceneExample()
{
}

PlayerSceneExample::~PlayerSceneExample()
{
}


IObject* sphereMove = nullptr;

void PlayerSceneExample::Tick(InputClass* const apInput, float aDT)
{
	//throw std::logic_error("The method or operation is not implemented.");
	static int x = 0;
	static int y = 0;

	
	int mouseRelX = 0;
	int mouseRelY = 0;

	if (apInput->rMouseDownC)
	{

		apInput->GetMouseRelativeLocation(mouseRelX, mouseRelY);
		//std::cout << mouseRelX << std::endl;

		x = mouseRelX;
		y = mouseRelY;


		mpCamera->Pitch(y / 360.0f);
		mpCamera->RotateY(x / 360.0f);
	}

	if (apInput->IsKeyHeld(0x53))
	{
		mpCamera->Walk(-1.0f * aDT);
	}
	
	if (apInput->IsKeyHeld(0x57))
	{
		mpCamera->Walk(1.0f * aDT);
	}
	
	if (apInput->IsKeyHeld(0x41))
	{
		mpCamera->Strafe(-1.0f* aDT);
	}
	
	if (apInput->IsKeyHeld(0x44))
	{
		mpCamera->Strafe(1.0f * aDT);
	}

	mpCamera->UpdateViewMatrix();
	
	
	tempT += 1.0f * aDT;
	//std::cout << tempT << std::endl;
	sphereMove->mPrevWorldMatrix = sphereMove->mWorldMatrix;
	sphereMove->mWorldMatrix = glm::transpose(glm::translate(glm::mat4(), glm::vec3(sin(tempT) * 2.0f, 3.0f, 1.0f)));

	mpSkyboxSphere->mWorldMatrix = glm::transpose(glm::translate(glm::mat4(), glm::vec3(mpCamera->GetPosition3f().x, mpCamera->GetPosition3f().y, mpCamera->GetPosition3f().z)));
	//mpSkyboxSphere->mWorldMatrix = glm::transpose(glm::scale(glm::mat4(1), glm::vec3(5.0f)));

	// Update camera frustum planes
	mpCamera->UpdateFrustumPlanes();

}

#include "MathHelper.h"
void PlayerSceneExample::Init()
{
	mpCamera->SetPosition(0.0f, 2.0f, -1.0f);
	
	
	std::vector<ModelID> tModels;
	std::vector<ModelID> modelSphere;

	
	XMMATRIX tScaleMat = tScaleMat = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX tTranslateMat = XMMatrixTranslation(0.0f, 0.0f, 0.0f);


	modelSphere = ResourceManager::GetInstance().LoadModels("Models\\Sphere\\Sphere.obj", false);

	for (int y = 0; y < 1; y++)
	{
		for (int x = 0; x < 1; x++)
		{
			tModels = ResourceManager::GetInstance().LoadModels("Models\\Sponza\\Sponza.obj", true);
			for (int i = 0; i < tModels.size(); ++i)
			{
				std::unique_ptr<IObject> tpObject = std::make_unique<IObject>();
				tpObject->mpModel = tModels[i];
				tpObject->mpMaterial = ResourceManager::GetInstance().GetModelByID(tModels[i])->material;
				Model* tMod = ResourceManager::GetInstance().GetModelByID(tModels[i]);
				tpObject->mWorldMatrix = glm::transpose(glm::translate(glm::mat4(), glm::vec3(x * 50.0f, 0.0f, y* 50.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.01f)));
				tpObject->mPrevWorldMatrix = tpObject->mWorldMatrix;


				glm::vec4 position = glm::vec4(tMod->sphereCollider.x, tMod->sphereCollider.y, tMod->sphereCollider.z, 1.0f);
				tpObject->mSpherePosition = glm::translate(glm::mat4(), glm::vec3(x*50.0f, 0.0f, y*50.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.01f)) * position;
				tpObject->mSpherePosition.w = tMod->sphereCollider.w * 0.01f;
		
				this->mObjects.push_back(std::move(tpObject));
			}
		}
	}

	// Prepare skybox sphere
	RawMeshData data;
	data.diffuseData.filepath = "textures\\sunsetcube1024.dds";
	data.diffuseData.isValid = true;

	data.specularData.isValid = false;
	data.normalData.isValid = false;

	Material* tM = ResourceManager::GetInstance().LoadCubeMapTexturesFromMaterial(data);
	tModels = ResourceManager::GetInstance().LoadModels("Models\\Sphere\\Sphere.obj", false);

	mpSkyboxSphere->mpModel = tModels[0];
	mpSkyboxSphere->mpMaterial = tM;


	this->sphereModelID = mpSkyboxSphere->mpModel.GetID();


	std::unique_ptr<IObject> sphereObject = std::make_unique<IObject>();
	sphereObject->mpModel = tModels[0];
	sphereMove = sphereObject.get();
	this->mObjects.push_back(std::move(sphereObject));
		
	// Create light, set diffuse and position, add light to list
	//std::unique_ptr<Light> tpLight = std::make_unique<Light>();
	//tpLight->diffuseColor = XMFLOAT3(1.0f, 0.0f, 1.0f);
	//tpLight->position = XMFLOAT3(0.0f, 2.0f, 0.0f);
	////
	//this->mLights.push_back(std::move(tpLight));


	mDirectionalLight = std::make_unique<LightData>();
	mDirectionalLight->diffuseCol = glm::vec3(1.0f, 1.0f, 1.0f);

	mDirectionalLight->position = glm::vec3(1.6, 2.6f, 0.0f);
	
	mDirectionalLight->dirVector.x = 0.0f;
	mDirectionalLight->dirVector.y = 0.0f;
	mDirectionalLight->dirVector.z = 0.0f;

	XMMATRIX ProjMatrix = XMMatrixOrthographicLH(50.0f, 50.0f, -50.0f, 50.0f);

	
	MathHelper::GenerateViewMatrixBasedOnDir(mDirectionalLight->position, mDirectionalLight->view, mDirectionalLight->dirVector);
	memcpy(&mDirectionalLight->proj[0], &ProjMatrix.r[0], sizeof(glm::mat4));




	//throw std::logic_error("The method or operation is not implemented.");
}

void PlayerSceneExample::Destroy()
{
	//throw std::logic_error("The method or operation is not implemented.");
}
