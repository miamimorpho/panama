#ifndef DUNGEON_H
#define DUNGEON_H

struct Dungeon{
    struct Space *space;
    struct Monsters *monsters;
};

void dungeonGenerate(struct Dungeon *);

#endif //DUNGEON_H
