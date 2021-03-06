// NGUIFontULTFontPlugin.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include <string>
#include <set>
#include <tuple>
using namespace std;

#include "AssetsTools/AssetsFileReader.h"
#include "AssetsTools/AssetsFileFormat.h"
#include "AssetsTools/ClassDatabaseFile.h"
#include "AssetsTools/AssetsFileTable.h"
#include "AssetsTools/ResourceManagerFile.h"
#include "AssetsTools/AssetTypeClass.h"
#include "IULTPluginCommonInterface.h"
#include "IULTFontPluginInterface.h"
#include "GeneralPurposeFunctions.h"
#include "json/json.h"

UnityL10nToolAPI* UnityL10nToolAPIGlobal;
FontPluginInfo* FontPluginInfoGlobal;

FontAssetMaps fontAssetMapsGlobal;
vector<AssetMapOption> OptionsList;
Json::Value OptionsJson;
Json::Value ProjectConfig;

bool _cdecl SetProjectConfigJson(Json::Value pluginConfig) {
	ProjectConfig = pluginConfig;
	return true;
}


class NGUIFontResource {
public:
	std::wstring FileName;
	std::wstring FontFamilyName;
	int m_Width;
	int m_Height;
	int m_CompleteImageSize;
	NGUIFontResource() {}
	NGUIFontResource(std::wstring FileName, Json::Value json);
	std::wstring GetMonoFileName();
	std::wstring GetResSFileName();
	//std::wstring GetAtlasFileName();
};

inline NGUIFontResource::NGUIFontResource(std::wstring FileName, Json::Value json) {
	this->FileName = FileName;
	this->FontFamilyName = WideMultiStringConverter->from_bytes(json["FontFamilyName"].asString());
	this->m_Width = json["m_Width"].asInt();
	this->m_Height = json["m_Height"].asInt();
	this->m_CompleteImageSize = json["m_CompleteImageSize"].asInt();
}

inline wstring NGUIFontResource::GetMonoFileName() {
	wstring monoFileName = this->FileName;
	ReplaceAll(monoFileName, L"Font.json", L"Mono.json");
	return monoFileName;
}

inline wstring NGUIFontResource::GetResSFileName() {
	wstring resSFileName = this->FileName;
	ReplaceAll(resSFileName, L"Font.json", L"assets.resS");
	return resSFileName;
}

/*inline wstring NGUIFontResource::GetAtlasFileName() {
	wstring AtlasFileName = this->FileName;
	ReplaceAll(AtlasFileName, L"Font.json", L"Atlas.json");
	return AtlasFileName;
}*/




string GetGameObjectName(AssetTypeValueField * pBase, AssetsFileTable * assetsFileTable);

tuple<INT32, INT64> GetUIAtlasFileIDPathID(AssetTypeValueField * monoBaseATVF, AssetsFileTable * assetsFileTable);

tuple<INT32, INT64> GetMaterialFilePathIDFromAtlas(AssetTypeValueField* monoBaseATVF, DWORD AssetsFileFormat);

tuple<INT32, INT64> GetMaterialFilePathIDFromMono(AssetTypeValueField* monoBaseATVF, DWORD AssetsFileFormat);

tuple<INT32, INT64> GetMaterialFilePathID(AssetTypeValueField* monoBaseATVF, DWORD AssetsFileFormat, string fieldName);

INT64 GetSetTexturePathID(AssetTypeValueField* MaterialBaseATVF, AssetsFileTable* assetsFileTable, INT64 pathID = 0);

AssetsReplacer* ReplaceTextureAsset(AssetsFileTable* assetsFileTable, const INT64& m_PathIDOfm_Texture, NGUIFontResource& localNGUIFontResource, const INT64 m_PathIDOfm_TextureTarget);

map<wstring, NGUIFontResource> NGUIFontResources;

string GetGameObjectName(AssetTypeValueField * pBase, AssetsFileTable * assetsFileTable)
{
	AssetTypeValueField* m_GameObjectATVF = pBase->Get("m_GameObject");
	if (m_GameObjectATVF && !m_GameObjectATVF->IsDummy()) {
		int m_FileID = m_GameObjectATVF->Get("m_FileID")->GetValue()->AsInt();
		INT64 m_PathID = m_GameObjectATVF->Get("m_PathID")->GetValue()->AsInt64();
		string gameObjectAssetsName = UnityL10nToolAPIGlobal->FindAssetsNameFromFileIdDependencies(m_FileID, assetsFileTable);
		map<string, AssetsFileTable*>::const_iterator gameObjectAFTItr = UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->find(gameObjectAssetsName);
		if (gameObjectAFTItr != UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->end()) {
			AssetsFileTable* gameObjectAFT = gameObjectAFTItr->second;
			AssetFileInfoEx* gameObjectAFIEx = gameObjectAFT->getAssetInfo(m_PathID);
			AssetTypeInstance* GameObjectATI = UnityL10nToolAPIGlobal->GetDetailAssetTypeInstanceFromAssetFileInfoEx(gameObjectAFT, gameObjectAFIEx);
			AssetTypeValueField* GameObjectATVF = GameObjectATI->GetBaseField();
			if (GameObjectATVF) {
				AssetTypeValueField* GameObject_m_NameATVF = GameObjectATVF->Get("m_Name");
				if (GameObject_m_NameATVF && !GameObject_m_NameATVF->IsDummy()) {
					string GameObject_m_Name = GameObject_m_NameATVF->GetValue()->AsString();
					return GameObject_m_Name;
				}
			}
		}
	}
	return string();
}

FontAssetMaps _cdecl GetPluginSupportAssetMap() {
	for (vector<string>::iterator assetsNameItr = UnityL10nToolAPIGlobal->AssetsFileNames->begin(); assetsNameItr != UnityL10nToolAPIGlobal->AssetsFileNames->end(); assetsNameItr++) {
		size_t found = assetsNameItr->find("sharedassets");
		size_t found2 = assetsNameItr->find("resources");
		if (found != string::npos || found2 != string::npos) {
			string assetsName = *assetsNameItr;
			// .UIFont 인지 UIFont인지 확인해봐야함
			vector<FontAssetMap> foundFontAssetMapList = GetFontAssetMapListFromMonoClassName(UnityL10nToolAPIGlobal, *assetsNameItr, ".UIFont");
			map<string, AssetsFileTable*>::const_iterator assetsFileTableItr = UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->find(assetsName);
			if (assetsFileTableItr != UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->end()) {
				AssetsFileTable* assetsFileTable = assetsFileTableItr->second;
				for (vector<FontAssetMap>::iterator FAMItr = foundFontAssetMapList.begin(); FAMItr != foundFontAssetMapList.end(); FAMItr++) {
					AssetFileInfoEx* assetFileInfoEx = assetsFileTable->getAssetInfo(FAMItr->_m_PathID);
					AssetTypeInstance* assetTypeInstance = UnityL10nToolAPIGlobal->GetDetailAssetTypeInstanceFromAssetFileInfoEx(assetsFileTable, assetFileInfoEx);
					AssetTypeValueField* pBase = assetTypeInstance->GetBaseField();
					if (pBase) {
						string GameObjectName = GetGameObjectName(pBase, assetsFileTable);
						if (GameObjectName != "") {
							FAMItr->Id = WideMultiStringConverter->from_bytes(GameObjectName);
							fontAssetMapsGlobal.News.push_back(*FAMItr);
						}
					}
				}
			}
		}
	}
	for (vector<FontAssetMap>::iterator fontAssetMapItr = fontAssetMapsGlobal.News.begin(); fontAssetMapItr != fontAssetMapsGlobal.News.end(); fontAssetMapItr++) {
		fontAssetMapItr->options = OptionsList;
	}
	/* Load Saveds and default configure */
	if (ProjectConfig.isMember("Saveds")) {
		Json::Value savedJsonArray = ProjectConfig["Saveds"];
		if (savedJsonArray.isArray()) {
			for (Json::ArrayIndex i = 0; i < savedJsonArray.size(); i++) {
				FontAssetMap savedFAM((Json::Value)savedJsonArray[i]);
				for (vector<FontAssetMap>::iterator fontAssetMapItr = fontAssetMapsGlobal.News.begin(); fontAssetMapItr != fontAssetMapsGlobal.News.end(); fontAssetMapItr++) {
					if (fontAssetMapItr->LooseEquals(savedFAM)) {
						wstring savedfontFamilyName = *(wstring*)savedFAM.options[0].Value;
						if (NGUIFontResources.find(savedfontFamilyName) != NGUIFontResources.end()) {
							fontAssetMapItr->options[0].Value = new wstring(savedfontFamilyName);
						}
						fontAssetMapItr->useContainerPath = savedFAM.useContainerPath;
						fontAssetMapsGlobal.Saveds.push_back(*fontAssetMapItr);
					}
				}
			}
		}
	}
	return fontAssetMapsGlobal;
}

Json::Value _cdecl GetProjectConfigJson() {
	Json::Value exportJson;

	Json::Value SavedsJsonArray(Json::arrayValue);
	for (vector<FontAssetMap>::iterator iterator = fontAssetMapsGlobal.Saveds.begin(); iterator != fontAssetMapsGlobal.Saveds.end(); iterator++) {
		SavedsJsonArray.append(iterator->ToJson());
	}
	exportJson["Saveds"] = SavedsJsonArray;
	return exportJson;
}

Json::Value _cdecl GetPatcherConfigJson() {
	return GetProjectConfigJson();
}

Json::Value PatcherConfigGlobal;
bool _cdecl SetPatcherConfigJson(Json::Value patcherConfig) {
	PatcherConfigGlobal = patcherConfig;
	return true;
}

bool _cdecl SetPluginSupportAssetMap(FontAssetMaps supportAssetMaps) {
	fontAssetMapsGlobal = supportAssetMaps;
	return true;
}

bool _cdecl CopyBuildFileToBuildFolder(wstring FontPluginRelativePath, wstring targetPath) {
	set<wstring> usedFontResources;
	for (vector<FontAssetMap>::iterator iterator = fontAssetMapsGlobal.Saveds.begin(); iterator != fontAssetMapsGlobal.Saveds.end(); iterator++) {
		wstring usedFontFamilyName = *(wstring*)iterator->options[0].Value;
		if (NGUIFontResources.find(usedFontFamilyName) != NGUIFontResources.end()) {
			usedFontResources.insert(usedFontFamilyName);
		}
	}
	CreateDirectoryW((targetPath + L"NGUIFontAsset\\").c_str(), NULL);
	for (set<wstring>::iterator iterator = usedFontResources.begin(); iterator != usedFontResources.end(); iterator++) {
		map<wstring, NGUIFontResource>::iterator NGUIFontResourceItr = NGUIFontResources.find(*iterator);
		if (NGUIFontResourceItr != NGUIFontResources.end()) {
			NGUIFontResource nGUIFontResource = NGUIFontResourceItr->second;
			CopyFileW((FontPluginRelativePath + L"NGUIFontAsset\\" + nGUIFontResource.FileName).c_str(), (targetPath + L"NGUIFontAsset\\" + nGUIFontResource.FileName).c_str(), false);
			CopyFileW((FontPluginRelativePath + L"NGUIFontAsset\\" + nGUIFontResource.GetMonoFileName()).c_str(), (targetPath + L"NGUIFontAsset\\" + nGUIFontResource.GetMonoFileName()).c_str(), false);
			//CopyFileW((FontPluginRelativePath + L"NGUIFontAsset\\" + nGUIFontResource.GetAtlasFileName()).c_str(), (targetPath + L"NGUIFontAsset\\" + nGUIFontResource.GetAtlasFileName()).c_str(), false);
			CopyFileW((FontPluginRelativePath + L"NGUIFontAsset\\" + nGUIFontResource.GetResSFileName()).c_str(), (targetPath + L"NGUIFontAsset\\" + nGUIFontResource.GetResSFileName()).c_str(), false);
		}
	}
	return true;
}


set<wstring> usedFontResourcesInPatcher;
void ReplaceStreamDataATVF(AssetTypeValueField * m_StreamDataATVF, NGUIFontResource &localNGUIFontResource)
{
	m_StreamDataATVF->Get("offset")->GetValue()->Set(new UINT32(0));
	m_StreamDataATVF->Get("size")->GetValue()->Set(new UINT32(localNGUIFontResource.m_CompleteImageSize));
	m_StreamDataATVF->Get("path")->GetValue()->Set((void*)(new string("NGUIFontAsset\\" + WideMultiStringConverter->to_bytes(localNGUIFontResource.GetResSFileName())))->c_str());
}
void ReplaceImageDataATVF(NGUIFontResource &localNGUIFontResource, AssetTypeValueField * TextureBaseATVF)
{
	wstring fileName = FontPluginInfoGlobal->relativePluginPath + L"NGUIFontAsset\\" + localNGUIFontResource.GetResSFileName();
	std::ifstream ifs(fileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	if (!ifs.good()) {
		throw new exception(("Cannot Find " + WideMultiStringConverter->to_bytes(fileName)).c_str());
	}
	//ifs.seekg(0, std::ios::beg);
	std::ifstream::pos_type fileSize = ifs.tellg();
	ifs.close();
	AssetTypeByteArray byteArrayData;
	BYTE* buffer = new BYTE[fileSize];
	HANDLE hFile = CreateFileW(
		fileName.c_str(),
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		throw new exception("INVALID_HANDLE_VALUE");
	}
	DWORD numberOfBytesToRead = 0;
	ReadFile(hFile, buffer, fileSize, &numberOfBytesToRead, NULL);
	CloseHandle(hFile);
	byteArrayData.size = fileSize;
	byteArrayData.data = buffer;
	AssetTypeValueField* imageDataATVF = TextureBaseATVF->Get("image data");
	imageDataATVF->GetValue()->Set(&byteArrayData);
}


/*AssetsReplacer* ReplaceAtalsAsset(AssetsFileTable* assetsFileTable, AssetFileInfoEx* AFIEx, AssetTypeInstance* MAtlasATI, NGUIFontResource &localNGUIFontResource) {
	string mAtlasJsonStr = readFile2(FontPluginInfoGlobal->relativePluginPath + L"NGUIFontAsset\\" + localNGUIFontResource.GetAtlasFileName());
	Json::Value mAtlasJson;
	JsonParseFromString(mAtlasJsonStr, mAtlasJson);
	return UnityL10nToolAPIGlobal->makeAssetsReplacer(assetsFileTable, AFIEx, MAtlasATI, mAtlasJson);
}*/

void ReplacePPtrFileID(AssetTypeValueField* pptrATVF, INT64 m_FileID) {
	pptrATVF->Get("m_FileID")->GetValue()->Set(new INT32(m_FileID));
}

void ReplacePPtrPathID(DWORD AssetsFileFormat, AssetTypeValueField* pptrATVF, INT64 m_PathID) {
	if (AssetsFileFormat < 0x10) {
		pptrATVF->Get("m_PathID")->GetValue()->Set(new INT32((INT32)m_PathID));
	}
	else {
		pptrATVF->Get("m_PathID")->GetValue()->Set(new INT64(m_PathID));
	}
}

void ReplacePPtrFilePathID(DWORD AssetsFileFormat, AssetTypeValueField* pptrATVF, INT32 m_FileID, INT64 m_PathID) {
	ReplacePPtrFileID(pptrATVF, m_FileID);
	ReplacePPtrPathID(AssetsFileFormat, pptrATVF, m_PathID);
}

map<string, vector<AssetsReplacer*>> _cdecl GetPatcherAssetReplacer() {
	map<string, vector<AssetsReplacer*>> replacers;
	FontAssetMaps fontAssetMaps = GetPluginSupportAssetMap();
	map<string,INT64> lastAssetsPathIDs;
	if (PatcherConfigGlobal.isMember("Saveds")) {
		Json::Value savedFontArrayJson = PatcherConfigGlobal["Saveds"];
		for (Json::ArrayIndex i = 0; i < savedFontArrayJson.size(); i++) {
			FontAssetMap fontAssetMap = FontAssetMap((Json::Value)savedFontArrayJson[i]);
			for (vector<FontAssetMap>::iterator FAMItr = fontAssetMaps.News.begin(); FAMItr != fontAssetMaps.News.end(); FAMItr++) {
				if (FAMItr->LooseEquals(fontAssetMap)) {
					/// Actual Make assetReplacer
					wstring fontFamilyName;
					if (fontAssetMap.options[0].Value != NULL) {
						try {
							fontFamilyName = *(wstring*)fontAssetMap.options[0].Value;
						}
						catch (exception ex) {

						}
					}
					map<wstring, NGUIFontResource>::iterator NGUIFontResourceItr = NGUIFontResources.find(fontFamilyName);
					if (NGUIFontResourceItr != NGUIFontResources.end()) {

						NGUIFontResource localNGUIFontResource = NGUIFontResourceItr->second;
						map<string, AssetsFileTable*>::const_iterator assetsFileTableItr = UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->find(fontAssetMap.assetsName);
						if (assetsFileTableItr != UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->end()) {
							AssetsFileTable* assetsFileTable = assetsFileTableItr->second;
							AssetFileInfoEx* MonoAFIEx = assetsFileTable->getAssetInfo(FAMItr->_m_PathID);
							AssetTypeInstance* MonoATI = UnityL10nToolAPIGlobal->GetDetailAssetTypeInstanceFromAssetFileInfoEx(assetsFileTable, MonoAFIEx);
							AssetTypeValueField* monoBaseATVF = MonoATI->GetBaseField();
							if (monoBaseATVF) {
								string monoJsonStr = readFile2(FontPluginInfoGlobal->relativePluginPath + L"NGUIFontAsset\\" + localNGUIFontResource.GetMonoFileName());
								Json::Value monoJson;
								JsonParseFromString(monoJsonStr, monoJson);

								tuple<INT32, INT64> m_FileIDPathIDOfmAtlas = GetUIAtlasFileIDPathID(monoBaseATVF, assetsFileTable);
								INT32 m_FileIDOfmAtlas = get<0>(m_FileIDPathIDOfmAtlas);
								INT64 m_PathIDOfmAtlas = get<1>(m_FileIDPathIDOfmAtlas);

								string assetsNameOfMaterial;
								AssetsFileTable* mMaterialAFT = nullptr;
								INT32 m_FileIDOfMaterial = -1;
								INT64 m_PathIDOfMaterial = -1;
								if (!(m_FileIDOfmAtlas == -1  && m_PathIDOfmAtlas == -1) &&
									!(m_FileIDOfmAtlas == 0 && m_PathIDOfmAtlas == 0)) {
									AssetsFileTable* mAtlasAFT;
									string mAtlasAssetsName;
									
									if (m_FileIDOfmAtlas == 0) {
										mAtlasAssetsName = fontAssetMap.assetsName;
										mAtlasAFT = assetsFileTable;
									}
									else {
										mAtlasAssetsName = string(assetsFileTable->getAssetsFile()->dependencies.pDependencies[m_FileIDOfmAtlas-1].assetPath);
										map<string, AssetsFileTable*>::const_iterator mAtlasAFTItr = UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->find(mAtlasAssetsName);
										mAtlasAFT = mAtlasAFTItr->second;
									}
									AssetFileInfoEx* MAtlasAFIEx = mAtlasAFT->getAssetInfo(m_PathIDOfmAtlas);
									AssetTypeInstance* MAtlasATI = UnityL10nToolAPIGlobal->GetDetailAssetTypeInstanceFromAssetFileInfoEx(mAtlasAFT, MAtlasAFIEx);
									AssetTypeValueField* MAtlasATVF = MAtlasATI->GetBaseField();
									if (MAtlasATVF) {
										tuple<INT32, INT64> m_FilePathIDOfMaterial = GetMaterialFilePathIDFromAtlas(MAtlasATVF, mAtlasAFT->getAssetsFile()->header.format);
										m_FileIDOfMaterial = get<0>(m_FilePathIDOfMaterial);
										m_PathIDOfMaterial = get<1>(m_FilePathIDOfMaterial);
										mMaterialAFT = mAtlasAFT;
										m_FileIDOfMaterial = m_FileIDOfmAtlas;
										assetsNameOfMaterial = mAtlasAssetsName;
									}
								}
								else {
									tuple<INT32, INT64> m_FilePathIDOfMaterial = GetMaterialFilePathIDFromMono(monoBaseATVF, assetsFileTable->getAssetsFile()->header.format);
									m_FileIDOfMaterial = get<0>(m_FilePathIDOfMaterial);
									m_PathIDOfMaterial = get<1>(m_FilePathIDOfMaterial);
									if (m_FileIDOfMaterial == 0) {
										mMaterialAFT = assetsFileTable;
										assetsNameOfMaterial = fontAssetMap.assetsName;
									}
									else {
										assetsNameOfMaterial = string(assetsFileTable->getAssetsFile()->dependencies.pDependencies[m_FileIDOfMaterial - 1].assetPath);
										map<string, AssetsFileTable*>::const_iterator mMaterialAFTItr = UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->find(assetsNameOfMaterial);
										mMaterialAFT = mMaterialAFTItr->second;
									}
								}
								if (mMaterialAFT != nullptr && m_PathIDOfMaterial != -1) {
									AssetFileInfoEx* mMaterialAFIEx = mMaterialAFT->getAssetInfo(m_PathIDOfMaterial);
									AssetTypeInstance* mMaterialATI = UnityL10nToolAPIGlobal->GetDetailAssetTypeInstanceFromAssetFileInfoEx(mMaterialAFT, mMaterialAFIEx);
									AssetTypeValueField* mMaterialATVF = mMaterialATI->GetBaseField();
									if (mMaterialATVF) {
										// 여기서 Material 에셋의 이름을 바꿔야 할지 고민
										INT64 m_PathIDOfTexture = GetSetTexturePathID(mMaterialATVF, mMaterialAFT);
										if (m_PathIDOfTexture != -1) {
											INT64 lastAssetsPathID = 0;
											if (lastAssetsPathIDs.find(fontAssetMap.assetsName) != lastAssetsPathIDs.end()) {
												lastAssetsPathID = lastAssetsPathIDs[fontAssetMap.assetsName];
											}
											else {
												lastAssetsPathID  = mMaterialAFT->pAssetFileInfo[mMaterialAFT->assetFileInfoCount - 1].index;
											}
											AssetTypeValueField* mMatATVF = monoBaseATVF->Get("mMat");
											if (mMatATVF && !mMatATVF->IsDummy()) {
												ReplacePPtrFilePathID(assetsFileTable->getAssetsFile()->header.format, mMatATVF, m_FileIDOfMaterial, lastAssetsPathID + 1);
												AssetTypeValueField* mAtlasATVF = monoBaseATVF->Get("mAtlas");
												if (mAtlasATVF && !mAtlasATVF->IsDummy()) {
													ReplacePPtrFilePathID(assetsFileTable->getAssetsFile()->header.format, mAtlasATVF, 0, 0);
												}
												AssetTypeValueField* mFontATVF = monoBaseATVF->Get("mFont");
												if (mFontATVF && !mFontATVF->IsDummy()) {
													mFontATVF->Get("mWidth")->GetValue()->Set(new INT32(localNGUIFontResource.m_Width));
													mFontATVF->Get("mHeight")->GetValue()->Set(new INT32(localNGUIFontResource.m_Height));
													AssetTypeValueField* mUVRectATVF = monoBaseATVF->Get("mUVRect");
													if (mUVRectATVF && !mUVRectATVF->IsDummy()) {
														mUVRectATVF->Get("x")->GetValue()->Set(new float(0));
														mUVRectATVF->Get("y")->GetValue()->Set(new float(0));
														mUVRectATVF->Get("width")->GetValue()->Set(new float(1));
														mUVRectATVF->Get("height")->GetValue()->Set(new float(1));
														AssetsReplacer* monoAssetsReplacer = UnityL10nToolAPIGlobal->makeAssetsReplacer(assetsFileTable, MonoAFIEx, MonoATI, monoJson);
														GetSetTexturePathID(mMaterialATVF, mMaterialAFT, lastAssetsPathID + 2);
														AssetsReplacer* TextureAssetsReplacer = ReplaceTextureAsset(mMaterialAFT, m_PathIDOfTexture, localNGUIFontResource, lastAssetsPathID + 2);
														AssetsReplacer * MaterialAssetsReplacer = UnityL10nToolAPIGlobal->makeAssetsReplacer(mMaterialAFT, mMaterialAFIEx, mMaterialATI, lastAssetsPathID + 1);
														replacers[fontAssetMap.assetsName].push_back(monoAssetsReplacer);
														replacers[assetsNameOfMaterial].push_back(MaterialAssetsReplacer);
														replacers[assetsNameOfMaterial].push_back(TextureAssetsReplacer);
														usedFontResourcesInPatcher.insert(fontFamilyName);
														lastAssetsPathIDs[fontAssetMap.assetsName] = lastAssetsPathID + 2;
													}
												}
											}
										}
									}
								}
							}
						}
					}
					break;
					/// ~Actual Make assetReplacer
				}
			}
		}
	}
	return replacers;
}

tuple<INT32, INT64> GetUIAtlasFileIDPathID(AssetTypeValueField * monoBaseATVF, AssetsFileTable * assetsFileTable) {
	AssetTypeValueField* mAtlasATVF = monoBaseATVF->Get("mAtlas");
	if (mAtlasATVF && !mAtlasATVF->IsDummy()) {
		AssetTypeValueField* m_PathIDATVF = mAtlasATVF->Get("m_PathID");
		AssetTypeValueField* m_FileIDATVF = mAtlasATVF->Get("m_FileID");
		if (m_PathIDATVF && !m_PathIDATVF->IsDummy() &&
			m_FileIDATVF && !m_FileIDATVF->IsDummy()) {
			return tuple<INT32, INT64>(m_FileIDATVF->GetValue()->AsInt(),m_PathIDATVF->GetValue()->AsInt64());
		}
	}
	return tuple<INT32, INT64>(-1,-1);
}

tuple<INT32, INT64> GetMaterialFilePathIDFromAtlas(AssetTypeValueField* monoBaseATVF, DWORD AssetsFileFormat) {
	return GetMaterialFilePathID(monoBaseATVF, AssetsFileFormat, "material");
}

tuple<INT32, INT64> GetMaterialFilePathIDFromMono(AssetTypeValueField* monoBaseATVF, DWORD AssetsFileFormat) {
	return GetMaterialFilePathID(monoBaseATVF, AssetsFileFormat, "mMat");
}

tuple<INT32, INT64> GetMaterialFilePathID(AssetTypeValueField* monoBaseATVF, DWORD AssetsFileFormat, string fieldName) {
	INT32 m_FileIDOfmMat = -1;
	INT64 m_PathIDOfmMat = -1;
	AssetTypeValueField* mMatATVF = monoBaseATVF->Get(fieldName.c_str());
	if (mMatATVF && !mMatATVF->IsDummy()) {
		AssetTypeValueField* m_FileIDOfmMayATVF = mMatATVF->Get("m_FileID");
		AssetTypeValueField* m_PathIDOfmMatATVF = mMatATVF->Get("m_PathID");
		if (m_PathIDOfmMatATVF && !m_PathIDOfmMatATVF->IsDummy() &&
			m_FileIDOfmMayATVF && !m_FileIDOfmMayATVF->IsDummy()) {
			m_FileIDOfmMat = m_FileIDOfmMayATVF->GetValue()->AsInt();
			if (AssetsFileFormat < 0x10) {
				m_PathIDOfmMat = (INT64)m_PathIDOfmMatATVF->GetValue()->AsInt();
			}
			else {
				m_PathIDOfmMat = m_PathIDOfmMatATVF->GetValue()->AsInt64();
			}
		}
	}
	return tuple<INT32, INT64>(m_FileIDOfmMat, m_PathIDOfmMat);
}

INT64 GetSetTexturePathID(AssetTypeValueField * MaterialBaseATVF, AssetsFileTable * assetsFileTable, INT64 pathID) {
	AssetTypeValueField* m_SavedPropertiesATVF = MaterialBaseATVF->Get("m_SavedProperties");
	if (m_SavedPropertiesATVF && !m_SavedPropertiesATVF->IsDummy()) {
		AssetTypeValueField* m_TexEnvsArrayATVF = m_SavedPropertiesATVF->Get("m_TexEnvs")->Get("Array");
		AssetTypeValueField** m_TexEnvsArrayPtrATVF = m_TexEnvsArrayATVF->GetChildrenList();
		for (DWORD i = 0; i < m_TexEnvsArrayATVF->GetChildrenCount(); i++) {
			AssetTypeValueField* m_texEnvChildATVF = m_TexEnvsArrayPtrATVF[i];
			AssetTypeValueField* nameOffirstATVF = m_texEnvChildATVF->Get("first");
			if (nameOffirstATVF && !nameOffirstATVF->IsDummy()) {
				string nameOffirst;
				if (nameOffirstATVF->GetChildrenCount() == 0) {
					nameOffirst = nameOffirstATVF->GetValue()->AsString();
				}
				else {
					AssetTypeValueField* firstNameOffirstATVF = m_texEnvChildATVF->Get("first")->Get("name");
					if (firstNameOffirstATVF && !firstNameOffirstATVF->IsDummy()) {
						nameOffirst = firstNameOffirstATVF->GetValue()->AsString();
					}
				}
				if (nameOffirst == "_MainTex") {
					AssetTypeValueField* m_PathIDOfm_TextureATVF = m_texEnvChildATVF->Get("second")->Get("m_Texture")->Get("m_PathID");
					if (m_PathIDOfm_TextureATVF && !m_PathIDOfm_TextureATVF->IsDummy()) {
						if (pathID != 0) {
							m_PathIDOfm_TextureATVF->GetValue()->Set(new INT64(pathID));
						}
						return m_PathIDOfm_TextureATVF->GetValue()->AsInt64();
					}
				}
			}		
		}
	}
	return -1;
}

AssetsReplacer* ReplaceTextureAsset(AssetsFileTable * assetsFileTable, const INT64 &m_PathIDOfm_Texture, NGUIFontResource &localNGUIFontResource, const INT64 m_PathIDOfm_TextureTarget)
{
	AssetFileInfoEx* TextureAFIEx = assetsFileTable->getAssetInfo(m_PathIDOfm_Texture);
	AssetTypeInstance* TextureATI = UnityL10nToolAPIGlobal->GetDetailAssetTypeInstanceFromAssetFileInfoEx(assetsFileTable, TextureAFIEx);
	AssetTypeValueField* TextureBaseATVF = TextureATI->GetBaseField();
	if (TextureBaseATVF) {
		TextureBaseATVF->Get("m_Width")->GetValue()->Set(new INT32(localNGUIFontResource.m_Width));
		TextureBaseATVF->Get("m_Height")->GetValue()->Set(new INT32(localNGUIFontResource.m_Height));
		TextureBaseATVF->Get("m_CompleteImageSize")->GetValue()->Set(new INT32(localNGUIFontResource.m_CompleteImageSize));
		TextureBaseATVF->Get("m_TextureFormat")->GetValue()->Set(new INT32(5));
		AssetTypeValueField* m_MipCountATVF = TextureBaseATVF->Get("m_MipCount");
		if (m_MipCountATVF && !m_MipCountATVF->IsDummy()) {
			m_MipCountATVF->GetValue()->Set(new INT32(1));
		}
		AssetTypeValueField* m_MipMapATVF = TextureBaseATVF->Get("m_MipMap");
		if (m_MipMapATVF && !m_MipMapATVF->IsDummy()) {
			m_MipMapATVF->GetValue()->Set(new BOOL(false));
		}
		AssetTypeValueField* m_StreamDataATVF = TextureBaseATVF->Get("m_StreamData");
		if (m_StreamDataATVF && !m_StreamDataATVF->IsDummy()) {
			ReplaceStreamDataATVF(m_StreamDataATVF, localNGUIFontResource);
		}
		else {
			ReplaceImageDataATVF(localNGUIFontResource, TextureBaseATVF);
		}
		return UnityL10nToolAPIGlobal->makeAssetsReplacer(assetsFileTable, TextureAFIEx, TextureATI, m_PathIDOfm_TextureTarget);
	}
}




bool _cdecl CopyResourceFileToGameFolder(wstring FontPluginRelativePath, wstring targetPath) {
	if (usedFontResourcesInPatcher.size() != 0) {
		CreateDirectoryW((targetPath + L"NGUIFontAsset\\").c_str(), NULL);
		for (set<wstring>::iterator iterator = usedFontResourcesInPatcher.begin(); iterator != usedFontResourcesInPatcher.end(); iterator++) {
			map<wstring, NGUIFontResource>::iterator NGUIFontResourceItr = NGUIFontResources.find(*iterator);
			if (NGUIFontResourceItr != NGUIFontResources.end()) {
				NGUIFontResource nGUIFontResource = NGUIFontResourceItr->second;
				CopyFileW((FontPluginRelativePath + L"NGUIFontAsset\\" + nGUIFontResource.GetResSFileName()).c_str(), (targetPath + L"NGUIFontAsset\\" + nGUIFontResource.GetResSFileName()).c_str(), false);
			}
		}
	}
	return true;
}

bool _cdecl GetFontPluginInfo(UnityL10nToolAPI* unityL10nToolAPI, FontPluginInfo* fontPluginInfo) {
	UnityL10nToolAPIGlobal = unityL10nToolAPI;
	FontPluginInfoGlobal = fontPluginInfo;
	wcsncpy_s(fontPluginInfo->FontPluginName, L"NGUIFontAsset", 13);
	fontPluginInfo->SetProjectConfigJson = SetProjectConfigJson;
	fontPluginInfo->GetPluginSupportAssetMap = GetPluginSupportAssetMap;
	fontPluginInfo->SetPluginSupportAssetMap = SetPluginSupportAssetMap;
	fontPluginInfo->GetProjectConfigJson = GetProjectConfigJson;
	fontPluginInfo->GetPacherConfigJson = GetPatcherConfigJson;
	fontPluginInfo->CopyBuildFileToBuildFolder = CopyBuildFileToBuildFolder;

	fontPluginInfo->SetPacherConfigJson = SetPatcherConfigJson;
	fontPluginInfo->GetPatcherAssetReplacer = GetPatcherAssetReplacer;
	fontPluginInfo->CopyResourceFileToGameFolder = CopyResourceFileToGameFolder;

	vector<wstring> NGUIFontNameList = get_all_files_names_within_folder(fontPluginInfo->relativePluginPath + L"NGUIFontAsset\\*.Font.json");
	AssetMapOption assetMapOption = AssetMapOption(
		L"Font Family",
		L"Select Font Family Name to use",
		NULL,
		NULL,
		AssetMapOption::Type::OPTION_TYPE_WSTRING,
		AssetMapOption::Type::OPTION_TYPE_NONE,
		vector<AssetMapOption>()
	);
	for (vector<wstring>::iterator iterator = NGUIFontNameList.begin(); iterator != NGUIFontNameList.end(); iterator++) {
		string NGUIFontJsonStr = readFile2(fontPluginInfo->relativePluginPath + L"NGUIFontAsset\\" + *iterator);
		Json::Value NGUIFontJson;
		JsonParseFromString(NGUIFontJsonStr, NGUIFontJson);
		NGUIFontResource nGUIFontResource(*iterator, NGUIFontJson);
		if (FileExist(fontPluginInfo->relativePluginPath + L"NGUIFontAsset\\" + nGUIFontResource.GetMonoFileName()) && FileExist(fontPluginInfo->relativePluginPath + L"NGUIFontAsset\\" + nGUIFontResource.GetResSFileName())) {
			NGUIFontResources.insert(pair<wstring, NGUIFontResource>(nGUIFontResource.FontFamilyName, nGUIFontResource));

			AssetMapOption assetMapOptionFontFamilyEnum = AssetMapOption(
				L"",
				L"",
				NULL,
				new wstring(nGUIFontResource.FontFamilyName),
				AssetMapOption::Type::OPTION_TYPE_NONE,
				AssetMapOption::Type::OPTION_TYPE_WSTRING,
				std::vector<AssetMapOption>()
			);
			assetMapOption.nestedOptions.push_back(assetMapOptionFontFamilyEnum);
		}
	}
	if (assetMapOption.nestedOptions.size() == 0) {
		AssetMapOption assetMapOptionFontFamilyEnum = AssetMapOption(
			L"",
			L"",
			NULL,
			new wstring(L"(Cannot load Font Resources."),
			AssetMapOption::Type::OPTION_TYPE_NONE,
			AssetMapOption::Type::OPTION_TYPE_WSTRING,
			std::vector<AssetMapOption>()
		);
	}
	OptionsList.push_back(assetMapOption);
	return true;
}