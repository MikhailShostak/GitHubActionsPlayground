#pragma once

#include "Config.hpp"

inline std::unordered_map<std::string, std::string> FileCache; //path, data
inline std::unordered_map<std::string, class Editor*> UnsavedFiles; //path, editor

std::string GetFileTitle(const std::filesystem::path &path);

bool LoadFile(const std::filesystem::path &path);
void ReloadFiles();

void OpenFile(const std::filesystem::path &path, bool remember = true);
void SaveFile(const std::filesystem::path &source, const std::filesystem::path &destination);
void SaveFile(const std::filesystem::path &path);
void CloseFile(const std::filesystem::path &path);
void CloseFiles(const std::vector<std::filesystem::path> &paths);
void CloseAllFiles();

void OpenFolder(const std::filesystem::path &path, bool remember = true);
void CloseFolder(const std::filesystem::path &path);
void CloseFolders(const std::vector<std::filesystem::path> &paths);
void CloseAllFolders();

void OpenPath(const std::filesystem::path &path, bool remember = true);
void OpenPaths(const std::vector<std::filesystem::path> &paths, bool remember = true);

std::string &GetFileContent(const std::filesystem::path &path);
