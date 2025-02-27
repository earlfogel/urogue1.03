/*
    init.c  -  global variable initialization
   
    UltraRogue
    Copyright (C) 1984, 1985, 1986, 1987, 1990 Herb Chong
    All rights reserved.
    
    Based on "Advanced Rogue"
    Copyright (C) 1983, 1984 Michael Morgan, Ken Dalka and AT&T
    All rights reserved.

    Based on "Super-Rogue"
    Copyright (C) 1982, 1983 Robert D. Kindelberger
    All rights reserved.

    Based on "Rogue: Exploring the Dungeons of Doom"
    Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
    All rights reserved.
    
    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include <ctype.h>
#include <string.h>
#include "rogue.h"

static char *rainbow[] = {
    "Red", "Blue", "Green", "Yellow",
    "Black", "Brown", "Orange", "Pink",
    "Purple", "Grey", "White", "Silver",
    "Gold", "Violet", "Clear", "Vermilion",
    "Ecru", "Turquoise", "Magenta", "Amber",
    "Topaz", "Plaid", "Tan", "Tangerine",
    "Aquamarine", "Scarlet", "Khaki", "Crimson",
    "Indigo", "Beige", "Lavender", "Saffron"
};

#define NCOLORS (sizeof rainbow / sizeof (char *))

static char *sylls[] = {
    "a", "ab", "ag", "aks", "ala", "an", "ankh", "app", "arg", "arze",
    "ash", "ban", "bar", "bat", "bek", "bie", "bin", "bit", "bjor",
    "blu", "bot", "bu", "byt", "comp", "con", "cos", "cre", "dalf",
    "dan", "den", "do", "e", "eep", "el", "eng", "er", "ere", "erk",
    "esh", "evs", "fa", "fid", "for", "fri", "fu", "gan", "gar",
    "glen", "gop", "gre", "ha", "he", "hyd", "i", "ing", "ion", "ip",
    "ish", "it", "ite", "iv", "jo", "kho", "kli", "klis", "la", "lech",
    "man", "mar", "me", "mi", "mic", "mik", "mon", "mung", "mur",
    "nej", "nelg", "nep", "ner", "nes", "nes", "nih", "nin", "o", "od",
    "ood", "org", "orn", "ox", "oxy", "pay", "pet", "ple", "plu", "po",
    "pot", "prok", "re", "rea", "rhov", "ri", "ro", "rog", "rok", "rol",
    "sa", "san", "sat", "see", "sef", "seh", "shu", "ski", "sna",
    "sne", "snik", "sno", "so", "sol", "sri", "sta", "sun", "ta",
    "tab", "tem", "ther", "ti", "tox", "trol", "tue", "turs", "u",
    "ulk", "um", "un", "uni", "ur", "val", "viv", "vly", "vom", "wah",
    "wed", "werg", "wex", "whon", "wun", "xo", "y", "yot", "yu",
    "zant", "zap", "zeb", "zim", "zok", "zon", "zum"
};

static char *stones[] = {
    "Agate", "Alexandrite", "Amethyst",
    "Azurite", "Carnelian", "Chrysoberyl",
    "Chrysoprase", "Citrine", "Diamond",
    "Emerald", "Garnet", "Hematite",
    "Jacinth", "Jade", "Kryptonite",
    "Lapus lazuli", "Malachite", "Moonstone",
    "Obsidian", "Olivine", "Onyx",
    "Opal", "Pearl", "Peridot",
    "Quartz", "Rhodochrosite", "Ruby",
    "Sapphire", "Sardonyx", "Serpentine",
    "Spinel", "Tiger eye", "Topaz",
    "Tourmaline", "Turquoise"
};

#define NSTONES (sizeof stones / sizeof (char *))

static char *wood[] = {
    "Avocado wood", "Balsa", "Banyan", "Birch",
    "Cedar", "Cherry", "Cinnibar", "Dogwood",
    "Driftwood", "Ebony", "Eucalyptus", "Hemlock",
    "Ironwood", "Mahogany", "Manzanita", "Maple",
    "Oak", "Pine", "Redwood", "Rosewood",
    "Teak", "Walnut", "Zebra wood", "Persimmon wood"
};

#define NWOOD (sizeof wood / sizeof (char *))

static char *metal[] = {
    "Aluminium", "Bone", "Brass", "Bronze",
    "Copper", "Chromium", "Iron", "Lead",
    "Magnesium", "Pewter", "Platinum", "Steel",
    "Tin", "Titanium", "Zinc"
};

#define NMETAL (sizeof metal / sizeof (char *))

const char *monstern = "monster";
char *spacemsg = "--Press SPACE to continue--";
char *morestr  = "--More--";
char *retstr   = "[Press RETURN to continue]";

/* 15 named levels */

const char *cnames[C_NOTSET][15] =
{
    {   "Veteran", "Warrior",
        "Swordsman", "Hero",    /* Fighter */
        "Swashbuckler", "Myrmidon",
        "Champion", "Superhero",
        "Lord", "Lord",
        "Lord", "Lord",
        "Lord", "Lord",
        "Lord"
    },

    {   "Gallant", "Keeper",
        "Protector", "Defender",    /* Paladin */
        "Warder", "Guardian",
        "Chevalier", "Justiciar",
        "Paladin", "Paladin",
        "Paladin", "Paladin",
        "Paladin", "Paladin",
        "Paladin"
    },

    {   "Runner", "Strider",
        "Scout", "Courser", /* Ranger */
        "Tracker", "Guide",
        "Pathfinder", "Ranger",
        "Ranger Knight", "Ranger Lord",
        "Ranger Lord", "Ranger Lord",
        "Ranger Lord", "Ranger Lord",
        "Ranger Lord"
    },

    {   "Acolyte", "Adept",
        "Priest", "Curate", /* Cleric */
        "Prefect", "Canon",
        "Lama", "Patriarch",
        "High Priest", "High Priest",
        "High Priest", "High Priest",
        "High Priest", "High Priest",
        "High Priest"
    },

    {   "Aspirant", "Ovate",   /* Druid */
        "Initiate of the 1st Circle", "Initiate of the 2nd Circle",
        "Initiate of the 3rd Circle", "Initiate of the 4th Circle",
        "Initiate of the 5th Circle", "Initiate of the 6th Circle",
        "Initiate of the 7th Circle", "Initiate of the 8th Circle",
        "Initiate of the 9th Circle", "Druid",
        "Archdruid", "The Great Druid",
        "The Grand Druid"
    },

    {   "Prestidigitator", "Evoker",
        "Conjurer", "Theurgist",    /* Magic User */
        "Thaumaturgist", "Magician",
        "Enchanter", "Warlock",
        "Sorcerer", "Necromancer",
        "Wizard", "Wizard",
        "Wizard", "Wizard",
        "Wizard"
    },

    {   "Prestidigitator", "Minor Trickster",
        "Trickster", "Master Trickster",    /* Illusionist */
        "Cabalist", "Visionist",
        "Phantasmist", "Apparitionist",
        "Spellbinder", "Illusionist",
        "Illusionist", "Illusionist",
        "Illusionist", "Illusionist",
        "Illusionist"
    },

    {   "Rogue", "Footpad",
        "Cutpurse", "Robber",   /* Thief */
        "Burglar", "Filcher",
        "Sharper", "Magsman",
        "Thief", "Master Thief",
        "Master Thief", "Master Thief",
        "Master Thief", "Master Thief",
        "Master Thief"
    },

    {   "Bravo", "Rutterkin",
        "Waghalter", "Murderer",    /* Assasin */
        "Thug", "Killer",
        "Cutthroat", "Executioner",
        "Assassin", "Expert Assassin",
        "Senior Assassin", "Chief Assassin",
        "Prime Assassin", "Guildmaster Assassin",
        "Grandfather of Assassins"
    },

    {   "Ninja", "Ninja",
        "Ninja", "Ninja",   /* Ninja */
        "Ninja", "Ninja",
        "Ninja", "Ninja",
        "Ninja", "Ninja",
        "Ninja", "Ninja",
        "Ninja", "Ninja",
        "Ninja"
    }
};

const struct h_list helpstr[] =
{
    { '?',      "  prints help"                         },
    { '/',      "  identify object"                     },
    { 'h',      "  left"                                },
    { 'j',      "  down"                                },
    { 'k',      "  up"                                  },
    { 'l',      "  right"                               },
    { 'y',      "  up & left"                           },
    { 'u',      "  up & right"                          },
    { 'b',      "  down & left"                         },
    { 'n',      "  down & right"                        },
    { '<',      "SHIFT><dir> run that way"              },
    { 'm',      "<dir> move onto without picking up"    },
    { 't',       "<dir> throw something"                },
    { 'z',      "<dir> zap a wand or staff"             },
    { '>',      "  go down a staircase"                 },
    { 's',      "  search for trap/secret door"         },
    { '.',      "  rest for a while"                    },
    { ',',      "  pick up an object"                   },
    { 'i',      "  inventory all items"                 },
    { 'I',      "  inventory type of item"              },
    { 'q',      "  quaff potion"                        },
    { 'r',      "  read paper"                          },
    { 'e',      "  eat food"                            },
    { 'w',      "  wield a weapon"                      },
    { 'W',      "  wear armor"                          },
    { 'T',      "  take armor off"                      },
    { 'P',      "  put on ring"                         },
    { 'R',      "  remove ring"                         },
    { 'A',      "  activate/apply an artifact"          },
    { 'd',      "  drop object"                         },
    { 'C',      "  call object (generic)"               },
    { 'M',      "  mark object (specific)"              },
    { 'o',      "  examine/set options"                 },
    { 'c',      "  cast a spell/say a prayer"           },
    { 'p',      "  pray for help (risky)"               },
    { 'a',      "  affect the undead"                   },
    { '^',      "  set a trap"                          },
    { 'D',      "  dip something (into a pool)"         },
    { 20,       "<dir>  take (steal) from (direction)"  }, /* ctrl-t */
    { 18,       "   redraw screen"                      }, /* ctrl-r */
    { 16,       "   back up to 10 previous messages"    }, /* ctrl-p */
    { ESCAPE,   "   cancel command"                     },
    { 'v',      "  print program version number"        },
    { 'S',      "  save game"                           },
    { 'Q',      "  quit"                                },
    { '=',      "  listen for monsters"                 },
    { 'f',      "<dir> fight monster"                   },
    { 'F',      "<dir> fight monster to the death"      },

    /* Wizard commands.  Identified by (h_ch != 0 && h_desc == 0). */

    {'-',       0                                       },
    { 23,       "   enter wizard mode"                  }, /* ctrl-w */
    { 23,       "v  toggle wizard verbose mode"         },
    { 23,       "e  exit wizard mode"                   },
    { 23,       "r  random number check"                },
    { 23,       "s  system statistics"                  },
    { 23,       "F  food statistics"                    },
    { 23,       "f  floor map"                          },
    { 23,       "m  see monster"                        },
    { 23,       "M  create monster"                     },
    { 23,       "c  create item"                        },
    { 23,       "i  inventory level"                    },
    { 23,       "I  identify item"                      },
    { 23,       "t  random teleport"                    },
    { 23,       "g  goto level"                         },
    { 23,       "C  charge item"                        },
    { 23,       "w  print worth of object"              },
    { 23,       "o  improve stats and pack"             },
    { 0,        0                                       }
};

/*
 * init_player: roll up the rogue
 */

void
init_player()
{
    bool    special = rnd(100) < 20;
    struct linked_list  *item;
    struct object   *obj;
    int which_armor, which_weapon;
    int other_weapon_flags = 0;

    pstats.s_lvl = 1;
    pstats.s_exp = 0L;
    pstats.s_arm = 10;

    if (!geta_player()) {
	do_getplayer(); /* get character class */
	pstats.s_str = 8 + rnd(5);
	pstats.s_intel = 8 + rnd(5);
	pstats.s_wisdom = 8 + rnd(5);
	pstats.s_dext = 8 + rnd(5);
	pstats.s_const = 10 + rnd(8);
	pstats.s_charisma = 8 + rnd(4);
	pstats.s_power = 5 + rnd(5);
	pstats.s_hpt = 15 + rnd(5);

	switch (char_type) {    /* Class-specific abilities */
	    when    C_FIGHTER:
		pstats.s_str = 17;
		pstats.s_const += rnd(4) + 1;
		if (special) {
		    pstats.s_str += rnd(3);
		    pstats.s_dext += rnd(4);
		}
		pstats.s_const = max(pstats.s_const, 16);
	    when    C_PALADIN:
		pstats.s_charisma = 17;
		if (special) {
		    pstats.s_charisma += rnd(3);
		    pstats.s_wisdom += rnd(4);
		    pstats.s_str += rnd(5);
		}
		pstats.s_str = max(pstats.s_str, 16);
		pstats.s_wisdom = max(pstats.s_wisdom, 16);
	    when    C_RANGER:
		if (special) {
		    pstats.s_wisdom += rnd(4);
		    pstats.s_intel += rnd(4);
		    pstats.s_str += rnd(5);
		}
		pstats.s_str = max(pstats.s_str, 16);
		pstats.s_wisdom = max(pstats.s_wisdom, 16);
		pstats.s_const = max(pstats.s_const, 14);
	    when    C_MAGICIAN:
		pstats.s_intel = (special) ? 18 : 16;
		pstats.s_power += 5 + rnd(10);
	    when    C_ILLUSION:
		pstats.s_intel = (special) ? 18 : 16;
		pstats.s_dext = (special) ? 18 : 14;
		pstats.s_power += 5 + rnd(10);
	    when    C_CLERIC:
		pstats.s_wisdom = (special) ? 18 : 16;
		pstats.s_str += rnd(4);
		pstats.s_power += 5 + rnd(10);
	    when    C_DRUID:
		if (special) {
		    pstats.s_wisdom += rnd(5);
		    pstats.s_charisma += rnd(4);
		}
		pstats.s_str += rnd(4);
		pstats.s_power += 5 + rnd(10);
		pstats.s_wisdom = max(pstats.s_wisdom, 16);
	    when    C_THIEF:
		pstats.s_dext = 18;
		if (special)
		    pstats.s_const += rnd(4) + 1;
	    when    C_ASSASIN:
		pstats.s_dext = (special) ? 18 : 16;
		pstats.s_intel = (special) ? 18 : 16;
	    when    C_NINJA:
		if (special) {
		    pstats.s_dext += rnd(5);
		    pstats.s_str += rnd(4);
		    pstats.s_intel += rnd(3);
		    pstats.s_wisdom += rnd(3);
		}
		pstats.s_dext = max(pstats.s_dext, 16);
		pstats.s_str = max(pstats.s_str, 15);
		pstats.s_wisdom = max(pstats.s_wisdom, 15);
		pstats.s_const = max(pstats.s_const, 15);
		pstats.s_charisma = max(pstats.s_charisma, 11);
	}
	puta_player();
    }
    if (char_type == C_ASSASIN || char_type == C_NINJA
	|| char_type == C_FIGHTER)
	pstats.s_dmg = "2d6";
    else
	pstats.s_dmg = "1d4";

    if (char_type == C_NINJA || char_type == C_FIGHTER)
	pstats.s_arm = 8;

    if (pstats.s_const > 15)
	pstats.s_hpt += pstats.s_const - 15;

    max_stats = pstats;
    player.t_rest_hpt = player.t_rest_pow = 0;
    pstats.s_carry = totalenc();
    pack = NULL;
    switch (player.t_ctype) {   /* now outfit pack */
	when    C_PALADIN:
	    purse = roll(20, 60);
	    which_armor = CHAIN_MAIL;
	    which_weapon = LONG_SWORD;
	when    C_FIGHTER:
	    purse = roll(50, 60);
	    which_armor = SCALE_MAIL;
	    which_weapon = BROAD_SWORD;
	when    C_RANGER:
	    purse = roll(50, 60);
	    which_armor = PADDED_ARMOR;
	    which_weapon = LONG_SPEAR;
	    other_weapon_flags |= ISOWNED | CANRETURN;
	when    C_CLERIC:
	    purse = roll(30, 80);
	    which_armor = RING_MAIL;
	    which_weapon = MORNINGSTAR;
	when    C_DRUID:
	    purse = roll(30, 80);
	    which_armor = STUDDED_LEATHER;
	    which_weapon = LIGHT_MACE;
	when    C_THIEF:
	    purse = roll(40, 80);
	    which_armor = HEAVY_LEATHER;
	    which_weapon = CUTLASS;
	when    C_ASSASIN:
	    purse = roll(20, 80);
	    which_armor = CUIRBOLILLI;
	    which_weapon = SABRE;
	    other_weapon_flags |= ISPOISON;
	when    C_NINJA:
	    purse = roll(20, 80);
	    which_armor = CUIRBOLILLI;
	    which_weapon = CRYSKNIFE;
	    other_weapon_flags |= ISPOISON;
	    item = spec_item(WEAPON, SHIRIKEN, 1, 1);
	    obj = OBJPTR(item);
	    obj->o_count = 1;
	    obj->o_flags |= ISKNOW | ISPOISON | ISOWNED | CANRETURN;
	    add_pack(item, NOMESSAGE);
	when    C_MAGICIAN:
	case C_ILLUSION:
	    purse = roll(20, 60);
	    which_armor = SOFT_LEATHER;
	    which_weapon = SINGLESTICK;
	otherwise: ;
    }

    /* Add weapon to pack */
    item = spec_item(WEAPON, which_weapon, 1, 1);
    obj = OBJPTR(item);
    obj->o_flags |= ISKNOW;
    obj->o_flags |= other_weapon_flags;
    obj->o_count = 1;
    add_pack(item, NOMESSAGE);
    cur_weapon = obj;

    /* Add armor to pack */
    item = spec_item(ARMOR, which_armor, 0, 0);
    obj = OBJPTR(item);
    obj->o_flags |= ISKNOW;
    obj->o_weight = armors[which_armor].a_wght;
    add_pack(item, NOMESSAGE);
    cur_armor = obj;

    /* Add some food to pack */
    item = spec_item(FOOD, FD_CRAM, 0, 0);
    obj = OBJPTR(item);
    obj->o_weight = things[TYP_FOOD].mi_wght;
    obj->o_count = 3;
    add_pack(item, NOMESSAGE);
}

/*
 * Initialize flags on startup
 */
void
init_flags()
{
    switch (player.t_ctype) {
    case C_MAGICIAN:
    case C_ILLUSION:
    case C_CLERIC:
    case C_DRUID:
	turn_on(player, CANSUMMON);
    otherwise: ;
    }
    turn_on(player, CANCAST);
    turn_on(player, CANWIELD);
}



/*
 * Contains definitions and functions for dealing with things like potions
 * and scrolls
 */

/*
 * init_things Initialize the probabilities for types of things
 */
void
init_things()
{
    register struct magic_item  *mp;

    for (mp = &things[1]; mp < &things[numthings]; mp++)
	mp->mi_prob += (mp - 1)->mi_prob;
    badcheck("things", things, numthings);
}

/*
 * init_fd Initialize the probabilities for types of food
 */
void
init_fd()
{
    register struct magic_item  *mp;

    for (mp = &fd_data[1]; mp < &fd_data[maxfoods]; mp++)
	mp->mi_prob += (mp - 1)->mi_prob;
    badcheck("food", fd_data, maxfoods);
}

/*
 * init_colors: Initialize the potion color scheme for this time
 */

void
init_colors()
{
    register int    i;
    register char   *str;

    for (i = 0; i < maxpotions; i++) {
	do {
	    str = rainbow[rnd(NCOLORS)];
	} until(isupper(*str));
	p_colors[i]    = strdup(str);
	p_colors[i][0] = (char) tolower(p_colors[i][0]);

	know_items[TYP_POTION][i] = FALSE;
	guess_items[TYP_POTION][i] = NULL;
	if (i > 0)
	    p_magic[i].mi_prob += p_magic[i - 1].mi_prob;
    }
    badcheck("potions", p_magic, maxpotions);
}

/*
 * init_names: Generate the names of the various scrolls
 */

void
init_names()
{
    register int    nsyl;
    register char   *cp, *sp;
    register int    i, nwords;

    for (i = 0; i < maxscrolls; i++) {
	cp = prbuf;
	nwords = rnd(ur_cols / 20) + 1 + (ur_cols > 40 ? 1 : 0);
	while (nwords--) {
	    nsyl = rnd(3) + 1;
	    while (nsyl--) {
		sp = sylls[rnd((sizeof sylls) /
		    (sizeof(char *)))];
		while (*sp)
		    *cp++ = *sp++;
	    }
	    *cp++ = ' ';
	}
	*--cp = '\0';
	s_names[i] = (char *) new_alloc(strlen(prbuf) + 1);
	know_items[TYP_SCROLL][i] = FALSE;
	guess_items[TYP_SCROLL][i] = NULL;
	strcpy(s_names[i], prbuf);
	if (i > 0)
	    s_magic[i].mi_prob += s_magic[i - 1].mi_prob;
    }
    badcheck("scrolls", s_magic, maxscrolls);
}

/*
 * init_stones: Initialize the ring stone setting scheme for this time
 */

void
init_stones()
{
    register int    i;
    register char   *str;

    for (i = 0; i < maxrings; i++) {
	do {
	    str = stones[rnd(NSTONES)];
	} until(isupper(*str));
	r_stones[i]    = strdup(str);
	r_stones[i][0] = (char) tolower( r_stones[i][0] );
	know_items[TYP_RING][i] = FALSE;
	guess_items[TYP_RING][i] = NULL;
	if (i > 0)
	    r_magic[i].mi_prob += r_magic[i - 1].mi_prob;
    }
    badcheck("rings", r_magic, maxrings);
}

/*
 * init_materials: Initialize the construction materials for wands and staffs
 */

void
init_materials()
{
    register int    i;
    register char   *str;

    for (i = 0; i < maxsticks; i++) {
	do {
	    if (rnd(100) > 50) {
		str = metal[rnd(NMETAL)];
		if (isupper(*str))
		    ws_type[i] = "wand";
	    }
	    else {
		str = wood[rnd(NWOOD)];
		if (isupper(*str))
		    ws_type[i] = "staff";
	    }
	} until(isupper(*str));
	ws_made[i] = strdup(str);
	ws_made[i][0] = (char) tolower(ws_made[i][0]);
	know_items[TYP_STICK][i] = FALSE;
	guess_items[TYP_STICK][i] = NULL;
	if (i > 0)
	    ws_magic[i].mi_prob += ws_magic[i - 1].mi_prob;
    }
    badcheck("sticks", ws_magic, maxsticks);
}

void
badcheck(char *name, struct magic_item *magic, int bound)
{
    register struct magic_item  *end;

    if (magic[bound - 1].mi_prob == 1000)
	return;
    printf("\nBad percentages for %s:\n", name);
    for (end = &magic[bound]; magic < end; magic++)
	printf("%3d%% %s\n", magic->mi_prob, magic->mi_name);
    printf("[Hit RETURN to continue]");
    fflush(stdout);
    while ((getchar() & 0177) != '\n')
	continue;
}
