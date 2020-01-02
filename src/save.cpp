// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "save.hpp"
#include <iostream>
#include <vector>
#include "Resources.hpp"

bool level_list_loaded = false;
bool sauvegarde_loaded = false;
int level_current = -1;
std::vector<std::string> level_name;

const char* max_level_filename = "/save";

int crypt(int n) {
  return (n + 1) * (n + 2) / 2 * 13 + 18;
}

int decrypt(int n) {
  int retour = 0;
  while (true) {
    if (crypt(retour) == n)
      return retour;
    else if (retour > 200)
      return -1;
    else
      retour++;
  };
}

void set_max_level(int n) {
  level_current = n;
  {
    std::ofstream file(SavePath() + max_level_filename);
    if (!file) {
      std::cerr << "failed to load level/sauvegarde" << std::endl;
      return;
    }
    
    file << crypt(n);
  }
  SyncFilesystem();
}

void load() {
  if (sauvegarde_loaded)
    return;
  sauvegarde_loaded = true;
  std::ifstream file(SavePath() + max_level_filename);
  std::string line;
  if (!file || !std::getline(file, line)) {
    level_current = -1;
    set_max_level(-1);
    return;
  }

  level_current = decrypt(std::stoi(line));
}

namespace save {

void level_completed(int n) {
  load();
  if (level_current >= n)
    return;
  set_max_level(n);
  level_current = n;
}

int max_level() {
  load();
  return level_current;
}

}  // namespace save
