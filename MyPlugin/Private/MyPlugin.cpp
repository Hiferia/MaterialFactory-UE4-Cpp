// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyPlugin.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "Engine/StaticMeshActor.h"
#include "Factories/MaterialFactoryNew.h"
#include "ImageUtils.h"
#include "LevelEditor.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Misc/MessageDialog.h"
#include "ScopedTransaction.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Misc/FileHelper.h"
#include "AutomatedAssetImportData.h"
#include "AssetToolsModule.h"
#include "HAL/FileManagerGeneric.h"

#define LOCTEXT_NAMESPACE "FMyPluginModule"

void FMyPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FMyPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

bool FMyPluginModule::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	const FString& TexturesPath = FString("C:/Users/luigi/Desktop/Textures");
	TArray<FString>ListOfTextures = TArray<FString>();
	FFileManagerGeneric FileManager = FFileManagerGeneric();
	FileManager.FindFilesRecursive(ListOfTextures, *TexturesPath, TEXT("*.png"), true, true);

	TArray<UObject*> Textures = ImportAsset(ListOfTextures);
	if (FParse::Command(&Cmd, TEXT("creatematerials")))
	{
		//Create material factory
		UMaterialFactoryNew* MyMaterialFactory = NewObject<UMaterialFactoryNew>();

		//Load file from path
		TArray<FString> Lines;
		FFileHelper::LoadFileToStringArray(Lines, Cmd);

		int32 NameIndex = 0;
		for (const FString& Line : Lines)
		{
			int32 Index = 0;
			FString MaterialName = FString("My_Mat_");
			MaterialName.AppendInt(NameIndex);

			TArray<FString> Values;
			Line.ParseIntoArray(Values, TEXT(" "));

			MaterialName = Values[Index];
			Index++;

			UPackage* Package = CreatePackage(*FString::Printf(TEXT("/Game/Materials/%s"), *MaterialName));

			UObject* ObjectMaterial = MyMaterialFactory->FactoryCreateNew(MyMaterialFactory->SupportedClass, Package, *MaterialName, EObjectFlags::RF_Standalone | EObjectFlags::RF_Public, nullptr, GWarn);

			FAssetRegistryModule::AssetCreated(ObjectMaterial);

			UMaterial* Material = Cast<UMaterial>(ObjectMaterial);
			Material->Modify();

			// BASE COLOR
			const FString& ColorCol = FString("Color");
			if (Values.Num() > Index && Values[Index].Equals(ColorCol, ESearchCase::IgnoreCase))
			{
				TArray<float> BaseColors = { 0, 0 ,0 };
				UMaterialExpressionVectorParameter* BaseColor = NewObject<UMaterialExpressionVectorParameter>(Material);
				Index++;
				BaseColors[0] = FCString::Atof(*Values[Index]);

				Index++;
				BaseColors[1] = FCString::Atof(*Values[Index]);

				Index++;
				BaseColors[2] = FCString::Atof(*Values[Index]);

				BaseColor->DefaultValue = FLinearColor(BaseColors[0], BaseColors[1], BaseColors[2]);
				BaseColor->ParameterName = TEXT("BaseColor");
				Material->BaseColor.Expression = BaseColor; //Collegare i nodi
				Material->Expressions.Add(BaseColor);
				Index++;
			}

			// ROUGHNESS
			const FString& RoughnessCol = FString("Roughness");
			if (Values.Num() > Index && Values[Index].Equals(RoughnessCol, ESearchCase::IgnoreCase))
			{
				float RoughnessValue = 0;
				UMaterialExpressionScalarParameter* Roughness = NewObject<UMaterialExpressionScalarParameter>(Material);
				Index++;
				RoughnessValue = FCString::Atof(*Values[Index]);
				Roughness->DefaultValue = RoughnessValue;
				Roughness->ParameterName = TEXT("Roughness");
				Material->Roughness.Expression = Roughness;
				Material->Expressions.Add(Roughness);
				Index++;
			}

			// SPECULAR
			const FString& SpecularCol = FString("Specular");
			if (Values.Num() > Index && Values[Index].Equals(SpecularCol, ESearchCase::IgnoreCase))
			{
				float SpecularValue = 0;
				UMaterialExpressionScalarParameter* Specular = NewObject<UMaterialExpressionScalarParameter>(Material);
				Index++;
				SpecularValue = FCString::Atof(*Values[Index]);
				Specular->DefaultValue = SpecularValue;
				Specular->ParameterName = TEXT("Specular");
				Material->Specular.Expression = Specular;
				Material->Expressions.Add(Specular);
				Index++;
			}
			// METALLIC
			const FString& MetallicCol = FString("Metallic");
			if (Values.Num() > Index && Values[Index].Equals(MetallicCol, ESearchCase::IgnoreCase))
			{
				float MetallicValue = 0;
				UMaterialExpressionScalarParameter* Metallic = NewObject<UMaterialExpressionScalarParameter>(Material);
				Index++;
				MetallicValue = FCString::Atof(*Values[Index]);
				Metallic->DefaultValue = MetallicValue;
				Metallic->ParameterName = TEXT("Metallic");
				Material->Metallic.Expression = Metallic;
				Material->Expressions.Add(Metallic);
				Index++;
			}

			// TEXTURE
			const FString& TextureCol = FString("Texture");
			if (Values.Num() > Index && Values[Index].Equals(TextureCol, ESearchCase::IgnoreCase))
			{
				int32 TextureIndex = 0;
				Index++;
				TextureIndex = FCString::Atof(*Values[Index]);
				UMaterialExpressionTextureSample* TextureExp = NewObject<UMaterialExpressionTextureSample>(Material);
				TextureExp->Texture = Cast<UTexture>(Textures[TextureIndex]);
				Material->Expressions.Add(TextureExp);
				Material->BaseColor.Expression = TextureExp;
			}

			Material->PostEditChange();
			Material->MarkPackageDirty();
		}
		return true;
	}
	return false;
}

TArray<UObject*> FMyPluginModule::ImportAsset(const TArray<FString>& Files)
{
	UAutomatedAssetImportData* NewTexture = NewObject<UAutomatedAssetImportData>();
	NewTexture->bReplaceExisting = true; //Replace old with new
	NewTexture->DestinationPath = TEXT("/Game/Assets/Textures");
	NewTexture->Filenames = Files;

	FAssetToolsModule& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
	TArray<UObject*>ImportedAssets = AssetTools.Get().ImportAssetsAutomated(NewTexture);

	for (UObject* Obj : ImportedAssets)
	{
		UPackage* Package = Obj->GetPackage();
		FString FileName = FPackageName::LongPackageNameToFilename(Package->GetPathName(), FPackageName::GetAssetPackageExtension());

		UPackage::SavePackage(Package, Obj, RF_Public | RF_Standalone, *FileName);
		FAssetRegistryModule::AssetCreated(Obj);
	}

	return ImportedAssets;
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMyPluginModule, MyPlugin)
