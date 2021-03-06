// RegExpULTTextPlugin.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//
// http://cinrueom.tistory.com/6
#pragma warning(disable: 4996)


#include "stdafx.h"
using namespace std;
#include <string>
#include <algorithm>
#include <regex>

#include "IULTPluginCommonInterface.h"
#include "IULTTextPluginInterface.h"
#include "GeneralPurposeFunctions.h"
#include "json/json.h"

TextPluginInfo* TextPluginInfoGlobal;

AssetMapOption ModifyKeyValueRegExpRecursive(AssetMapOption assetMapOptionFrom, int i, wstring keyValue);
wstring RegExpReplaceRecursive(wstring str, AssetMapOption assetMapOptionFrom);
LanguagePairDics _cdecl GetOriginalMapFromText(wstring OriginalText, LanguagePairDics languagePairDics) {
	if (languagePairDics.size() == 0) {
		languagePairDics.insert(pair<wstring,LanguagePairDic>(L"0", LanguagePairDic()));
	}
	for (map<wstring, LanguagePairDic>::iterator lpdItr = languagePairDics.begin();
		lpdItr != languagePairDics.end(); lpdItr++) {
		if (lpdItr->second.InteractWithAssetOptions.size() != 7) {
			lpdItr->second.InteractWithAssetOptions.clear();
			AssetMapOption firstRegExp = AssetMapOption(
				L"RegExp Match expression",
				L"",
				NULL,
				NULL,
				AssetMapOption::OPTION_TYPE_WSTRING,
				AssetMapOption::OPTION_TYPE_NONE,
				vector<AssetMapOption>()
			);
			AssetMapOption keySelectOption = AssetMapOption(
				L"Select Key Group",
				L"Select which group is key.",
				NULL,
				NULL,
				AssetMapOption::OPTION_TYPE_INT,
				AssetMapOption::OPTION_TYPE_NONE,
				vector<AssetMapOption>()
			);
			AssetMapOption valueSelectOption = AssetMapOption(
				L"Select Value Group",
				L"Select which group is value.",
				NULL,
				NULL,
				AssetMapOption::OPTION_TYPE_INT,
				AssetMapOption::OPTION_TYPE_NONE,
				vector<AssetMapOption>()
			);
			AssetMapOption keyRexExpReplaceFrom = AssetMapOption(
				L"Key Replace [0] From",
				L"",
				new wstring(L""),
				NULL,
				AssetMapOption::OPTION_TYPE_WSTRING,
				AssetMapOption::OPTION_TYPE_NONE,
				vector<AssetMapOption>()
			);
			AssetMapOption keyRexExpReplaceTo = AssetMapOption(
				L"Key Replace [0] To",
				L"",
				new wstring(L""),
				NULL,
				AssetMapOption::OPTION_TYPE_WSTRING,
				AssetMapOption::OPTION_TYPE_NONE,
				vector<AssetMapOption>()
			);
			keyRexExpReplaceFrom.nestedOptions.push_back(keyRexExpReplaceTo);

			AssetMapOption valueRexExpReplaceFrom = AssetMapOption(
				L"Value Replace [0] From",
				L"",
				new wstring(L""),
				NULL,
				AssetMapOption::OPTION_TYPE_WSTRING,
				AssetMapOption::OPTION_TYPE_NONE,
				vector<AssetMapOption>()
			);
			AssetMapOption valueRexExpReplaceTo = AssetMapOption(
				L"Value Replace [0] To",
				L"",
				new wstring(L""),
				NULL,
				AssetMapOption::OPTION_TYPE_WSTRING,
				AssetMapOption::OPTION_TYPE_NONE,
				vector<AssetMapOption>()
			);
			valueRexExpReplaceFrom.nestedOptions.push_back(valueRexExpReplaceTo);

			AssetMapOption valueRexExpReplaceReverseFrom = AssetMapOption(
				L"Value Replace Reserve [0] From",
				L"",
				new wstring(L""),
				NULL,
				AssetMapOption::OPTION_TYPE_WSTRING,
				AssetMapOption::OPTION_TYPE_NONE,
				vector<AssetMapOption>()
			);
			AssetMapOption valueRexExpReplaceReverseTo = AssetMapOption(
				L"Value Replace Reserve [0] To",
				L"",
				new wstring(L""),
				NULL,
				AssetMapOption::OPTION_TYPE_WSTRING,
				AssetMapOption::OPTION_TYPE_NONE,
				vector<AssetMapOption>()
			);
			valueRexExpReplaceReverseFrom.nestedOptions.push_back(valueRexExpReplaceReverseTo);


			AssetMapOption reverseRexExp = AssetMapOption(
				L"Reverse RegExp replace expression",
				L"$1 is key and $2 is value",
				new wstring(L""),
				NULL,
				AssetMapOption::OPTION_TYPE_WSTRING,
				AssetMapOption::OPTION_TYPE_NONE,
				vector<AssetMapOption>()
			);

			lpdItr->second.InteractWithAssetOptions.push_back(firstRegExp); // 0
			lpdItr->second.InteractWithAssetOptions.push_back(keySelectOption); // 1
			lpdItr->second.InteractWithAssetOptions.push_back(valueSelectOption); // 2
			lpdItr->second.InteractWithAssetOptions.push_back(keyRexExpReplaceFrom); // 3
			lpdItr->second.InteractWithAssetOptions.push_back(valueRexExpReplaceFrom); // 4
			lpdItr->second.InteractWithAssetOptions.push_back(valueRexExpReplaceReverseFrom); // 5
			lpdItr->second.InteractWithAssetOptions.push_back(reverseRexExp); // 6
		} else {
			AssetMapOption firstRegExp = lpdItr->second.InteractWithAssetOptions[0];
			if (firstRegExp.Value && *(wstring*)firstRegExp.Value != L"") {
				wstring regexression = *(wstring*)firstRegExp.Value;
				wregex firstRegex(regexression);
				wsregex_iterator it(OriginalText.begin(), OriginalText.end(), firstRegex);
				if (it != wsregex_iterator()) {
					if ((*it).size() - 1 != lpdItr->second.InteractWithAssetOptions[1].nestedOptions.size()) {
						lpdItr->second.InteractWithAssetOptions[1].nestedOptions.clear();
						for (unsigned int i = 1; i < (*it).size(); i++) {
							AssetMapOption keySelectOptionChild = AssetMapOption(
								L"",
								L"",
								NULL,
								new int(i),
								AssetMapOption::OPTION_TYPE_NONE,
								AssetMapOption::OPTION_TYPE_INT,
								vector<AssetMapOption>()
							);
							lpdItr->second.InteractWithAssetOptions[1].nestedOptions.push_back(keySelectOptionChild);
						}
						lpdItr->second.InteractWithAssetOptions[2].nestedOptions.clear();
						for (unsigned int i = 1; i < (*it).size(); i++) {
							AssetMapOption valueSelectOptionChild = AssetMapOption(
								L"",
								L"",
								NULL,
								new int(i),
								AssetMapOption::OPTION_TYPE_NONE,
								AssetMapOption::OPTION_TYPE_INT,
								vector<AssetMapOption>()
							);
							lpdItr->second.InteractWithAssetOptions[2].nestedOptions.push_back(valueSelectOptionChild);
						}
					}
					lpdItr->second.InteractWithAssetOptions[3] = ModifyKeyValueRegExpRecursive(lpdItr->second.InteractWithAssetOptions[3], 0, L"Key Replace");
					lpdItr->second.InteractWithAssetOptions[4] = ModifyKeyValueRegExpRecursive(lpdItr->second.InteractWithAssetOptions[4], 0, L"Value Replace");
					AssetMapOption keyRexExpReplaceFrom = lpdItr->second.InteractWithAssetOptions[3];
					AssetMapOption keyRexExpReplaceTo = keyRexExpReplaceFrom.nestedOptions[0];
					AssetMapOption valueRexExpReplaceFrom = lpdItr->second.InteractWithAssetOptions[4];
					AssetMapOption valueRexExpReplaceTo = keyRexExpReplaceFrom.nestedOptions[0];
					// 
					AssetMapOption keySelectOption = lpdItr->second.InteractWithAssetOptions[1];
					AssetMapOption valueSelectOption = lpdItr->second.InteractWithAssetOptions[2];
					if (keySelectOption.Value && valueSelectOption.Value &&
						*(int*)keySelectOption.Value != *(int*)valueSelectOption.Value) {
						for (/*wsregex_iterator it(OriginalText.begin(), OriginalText.end(), firstRegex)*/;
							it != wsregex_iterator(); ++it) {
							wstring key = (*it)[*(int*)keySelectOption.Value].str();
							wstring value = (*it)[*(int*)valueSelectOption.Value].str();
							key = RegExpReplaceRecursive(key, keyRexExpReplaceFrom);
							value = RegExpReplaceRecursive(value, valueRexExpReplaceFrom);
							if (key != L"" && value != L"") {
								LanguagePair lp;
								lp.Original = value;
								lpdItr->second.Dic[key] = lp;
							}
						}
					}
				}
			}
			lpdItr->second.InteractWithAssetOptions[5] = ModifyKeyValueRegExpRecursive(lpdItr->second.InteractWithAssetOptions[5], 0, L"Value Replace Reverse");
			
		}
	}
	return languagePairDics;
}

wstring RegExpReplaceRecursive(wstring str, AssetMapOption assetMapOptionFrom) {
	AssetMapOption assetMapOptionTo = assetMapOptionFrom.nestedOptions[0];
	if (assetMapOptionFrom.Value) {
		wstring wstrFrom = *(wstring*)assetMapOptionFrom.Value;
		if (!wstrFrom.empty()) {
			wstring toStr;
			if (assetMapOptionTo.Value) {
				toStr = *(wstring*)assetMapOptionTo.Value;
			}
			// negative lookbehind not work
			/*wregex replaceN(L"(?<!(\\\\))\\\\n");
			toStr = regex_replace(toStr, replaceN, wstring(L"\n"));
			wregex replaceT(L"(?<!(\\\\))\\\\t");
			toStr = regex_replace(toStr, replaceT, wstring(L"\t"));*/
			wregex replaceN(L"(.*?)(\\\\+n)");
			wsregex_iterator replaceNIt(toStr.begin(), toStr.end(), replaceN);
			wstring tempToStr;
			for (; replaceNIt != wsregex_iterator(); replaceNIt++) {
				if ((*replaceNIt)[2] == L"\\n") {
					tempToStr += wstring((*replaceNIt)[1]) + L"\n";
				}
				else {
					tempToStr += wstring((*replaceNIt)[1]) + wstring((*replaceNIt)[2]);
				}
			}
			if (tempToStr != L"") {
				toStr = tempToStr;
			}
			wregex replaceT(L"(.*?)(\\\\+t)");
			wsregex_iterator replaceTIt(toStr.begin(), toStr.end(), replaceT);
			tempToStr = L"";
			for (; replaceTIt != wsregex_iterator(); replaceTIt++) {
				if ((*replaceTIt)[2] == L"\\t") {
					tempToStr += wstring((*replaceTIt)[1]) + L"\t";
				}
				else {
					tempToStr += wstring((*replaceTIt)[1]) + wstring((*replaceTIt)[2]);
				}
			}
			if (tempToStr != L"") {
				toStr = tempToStr;
			}
			toStr = ReplaceAll(toStr, L"\\\\", L"\\");
			wregex wrgx(wstrFrom);
			for (wsregex_iterator it(str.begin(), str.end(), wrgx);
				it != wsregex_iterator(); it++) {
				int groupSize = (*it).size();
				wstring modifiedStr = toStr;
				for (int i = 1; i < groupSize; i++) {
					wregex wrgx1(L"\\$" + to_wstring((long long)i));
					wstring tempStr = (*it)[i];
					modifiedStr = regex_replace(modifiedStr, wrgx1, tempStr);
				}
				str = ReplaceAll(str, (*it)[0], modifiedStr);
			}
			//str = regex_replace(str, wrgx, toStr);
			if (assetMapOptionTo.nestedOptions.size() == 1) {
				str = RegExpReplaceRecursive(str, assetMapOptionTo.nestedOptions[0]);
			}
		}
	}
	return str;
}

AssetMapOption ModifyKeyValueRegExpRecursive(AssetMapOption assetMapOptionFrom, int i, wstring message) {
	AssetMapOption assetMapOptionTo = assetMapOptionFrom.nestedOptions[0];
	if (assetMapOptionFrom.Value && *(wstring*)assetMapOptionFrom.Value != L"") {
		if (assetMapOptionTo.nestedOptions.size() == 0) {
			AssetMapOption childFrom = AssetMapOption(
				message + L" [" + to_wstring((long long)++i) + L"] From",
				L"",
				new wstring(L""),
				NULL,
				AssetMapOption::OPTION_TYPE_WSTRING,
				AssetMapOption::OPTION_TYPE_NONE,
				vector<AssetMapOption>()
			);
			AssetMapOption childTo = AssetMapOption(
				message + L" [" + to_wstring((long long)i) + L"] To",
				L"",
				new wstring(L""),
				NULL,
				AssetMapOption::OPTION_TYPE_WSTRING,
				AssetMapOption::OPTION_TYPE_NONE,
				vector<AssetMapOption>()
			);
			childFrom.nestedOptions.push_back(childTo);
			assetMapOptionTo.nestedOptions.push_back(childFrom);
		}
		else {
			assetMapOptionTo.nestedOptions[0] = ModifyKeyValueRegExpRecursive(assetMapOptionTo.nestedOptions[0], ++i, message);
		}

	}
	assetMapOptionFrom.nestedOptions[0] = assetMapOptionTo;
	return assetMapOptionFrom;
}

wstring RegExpReplaceReverseRecursive(wstring str, AssetMapOption assetMapOptionFrom);
wstring _cdecl GetTranslatedTextFromMap(wstring OriginalText, LanguagePairDics TranslatedMap) {
	for (LanguagePairDics::iterator lpdItr = TranslatedMap.begin();
		lpdItr != TranslatedMap.end(); lpdItr++) {
		vector<pair<wstring, wstring>> modifyList;
		AssetMapOption firstRegExp = lpdItr->second.InteractWithAssetOptions[0];
		AssetMapOption keySelectOption = lpdItr->second.InteractWithAssetOptions[1];
		AssetMapOption valueSelectOption = lpdItr->second.InteractWithAssetOptions[2];
		AssetMapOption keyRegExpReplaceFrom = lpdItr->second.InteractWithAssetOptions[3];
		AssetMapOption valueRegExpReplaceFrom = lpdItr->second.InteractWithAssetOptions[4];
		AssetMapOption valueRexExpReplaceReverseFrom = lpdItr->second.InteractWithAssetOptions[5];
		AssetMapOption reverseRexExp = lpdItr->second.InteractWithAssetOptions[6];
		if (firstRegExp.Value && keySelectOption.Value && keyRegExpReplaceFrom.Value && valueRegExpReplaceFrom.Value && valueRexExpReplaceReverseFrom.Value && reverseRexExp.Value) {
			wstring regStr = *(wstring*)firstRegExp.Value;
			int keySelect = *(int*)keySelectOption.Value;
			int valueSelect = *(int*)valueSelectOption.Value;
			wstring regReverseStr = *(wstring*)reverseRexExp.Value;
			if (regStr != L"" && keySelect != valueSelect && regReverseStr != L"") {
				wregex wrgx(regStr);
				for (wsregex_iterator it(OriginalText.begin(), OriginalText.end(), wrgx);
					it != wsregex_iterator(); it++) {
					wstring originalBlock = (*it)[0];
					wstring key = (*it)[keySelect];
					wstring value = (*it)[valueSelect];
					wstring keyParsered = RegExpReplaceRecursive(key, keyRegExpReplaceFrom);
					map<wstring, LanguagePair>::iterator lpItr = lpdItr->second.Dic.find(keyParsered);
					if (lpItr != lpdItr->second.Dic.end()) {
						if (lpItr->second.Translated != L"") {
							//여기서 OriginalText를 변경
							wstring TranslatedReversedParsed = RegExpReplaceRecursive(lpItr->second.Translated, valueRexExpReplaceReverseFrom);
							wregex keyRegex(L"\\$1");
							wstring ModifiedBlock = regex_replace(regReverseStr, keyRegex, keyParsered);
							wregex valueRegex(L"\\$2");
							ModifiedBlock = regex_replace(ModifiedBlock, valueRegex, TranslatedReversedParsed);
							pair<wstring, wstring> modifyPair = pair<wstring, wstring>(wstring((*it)[0]), ModifiedBlock);
							modifyList.push_back(modifyPair);
							//OriginalText = ReplaceAll(OriginalText, wstring((*it)[0]), ModifiedBlock);
						}
					}
				}
				for (vector<pair<wstring, wstring>>::iterator iterator = modifyList.begin();
					iterator != modifyList.end(); iterator++) {
					ReplaceAll(OriginalText, iterator->first, iterator->second);
				}
			}
		}
	}
	return OriginalText;
}

wstring RegExpReplaceReverseRecursive(wstring str, AssetMapOption assetMapOptionFrom) {
	AssetMapOption assetMapOptionTo = assetMapOptionFrom.nestedOptions[0];
	if (assetMapOptionFrom.Value && assetMapOptionTo.Value && *(wstring*)assetMapOptionFrom.Value != L"" && *(wstring*)assetMapOptionTo.Value != L"") {
		str = RegExpReplaceReverseRecursive(str, assetMapOptionTo.nestedOptions[0]);
		wregex wrgx(*(wstring*)assetMapOptionTo.Value);
		return str = regex_replace(str, wrgx, *(wstring*)assetMapOptionFrom.Value);
	}
	else {
		return str;
	}
}

//
//LanguagePairDics _cdecl GetUpdateFileTextFromMap(LanguagePairDics UpdateMap) {
//
//}
//
//LanguagePairDics _cdecl GetTranslatedMapFromFileText(LanguagePairDics translatedMap) {
//
//}

bool _cdecl GetTextPluginInfo(TextPluginInfo* textPluginInfo) {
	TextPluginInfoGlobal = textPluginInfo;

	wcsncpy_s(textPluginInfo->TextPluginName, L"RegExp", 6);
	textPluginInfo->GetOriginalMapFromText = GetOriginalMapFromText;
	textPluginInfo->GetTranslatedTextFromMap = GetTranslatedTextFromMap;

	//textPluginInfo->GetUpdateFileTextFromMap = GetUpdateFileTextFromMap;
	//textPluginInfo->GetTranslatedMapFromFileText = GetTranslatedMapFromFileText;
	return true;
}