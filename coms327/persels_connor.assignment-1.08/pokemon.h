#ifndef POKEMON_H
# define POKEMON_H

# include <iostream>

enum pokemon_stat {
  stat_maxhp,
  stat_atk,
  stat_def,
  stat_spatk,
  stat_spdef,
  stat_speed
};

enum pokemon_gender {
  gender_female,
  gender_male
};

class Pokemon {
 private:
  int pokemon_index;
  int pokemon_species_index;
  int IV[6];
  int effective_stat[6];
  bool shiny;
  pokemon_gender gender;
 public:
  int move_index[4];
  int currenthp;
  int level;
  Pokemon(int level);
  const char *get_species() const;
  int get_maxhp() const;
  int get_atk() const;
  int get_def() const;
  int get_spatk() const;
  int get_spdef() const;
  int get_speed() const;
  const char *get_gender_string() const;
  bool is_shiny() const;
  const char *get_move(int i) const;
  std::ostream &print(std::ostream &o) const;
};

std::ostream &operator<<(std::ostream &o, const Pokemon &p);

#endif
