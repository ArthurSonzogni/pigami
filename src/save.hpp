// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <fstream>
#include <list>
#include <string>

#ifndef SAUVEGARDE
#define SAUVEGARDE

namespace save {

void level_completed(int n);
int max_level();

//std::string sauvegarde_name_of_level(int n);

}  // namespace save

//struct generated_level {
  //// the name of the level
  //std::string name;

  //// the minimal number of movement to solve the level
  //int min_cost;

  //// the best record of the player
  //int cost;

  //bool isModified;
  //generated_level();
  //generated_level(std::string n, int c, int cc);
//};

//std::list<generated_level> sauvegarde_generated_get_list();
//void sauvegarde_generated_save_list(std::list<generated_level> l);

#endif /* end of include guard: SAUVEGARDE */
