/*
    move.c  -  Hero movement commands
   
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
#include "rogue.h"

/*
 * Used to hold the new hero position
 */

coord   nh;

/*
 * do_run: Start the hero running
 */

void
do_run(char ch)
{
    running = TRUE;
    after = FALSE;
    runch = ch;
    if (doorstop && !on(player, ISBLIND)) {
	door_stop = TRUE;
	firstmove = TRUE;
    }
}

/*
 * corr_move: Check to see that a move is legal.  If so, return correct
 * character. If not, if player came from a legal place, then try to turn
 * him.
 */

void
corr_move(int dy, int dx)
{
    char    ch;
    short   legal = 0;  /* Number of legal alternatives */
    int y, x;       /* Holds legal new position */
    int *ny, *nx;   /* Point to which direction to change */

    /* New position */
    nh.y = hero.y + dy;
    nh.x = hero.x + dx;

    /* A bad diagonal move is illegal */
    if (!diag_ok(&hero, &nh, &player))
	return;

    /* If it is a legal move, just return */
    if (nh.x >= 0 && nh.x < ur_cols && nh.y > 0 && nh.y < ur_lines - 2) {
	ch = winat(nh.y, nh.x);
	switch (ch) {
	    case ' ':
	    case '|':
	    case '-':
		break;
	    default:
		return;
	}
    }

    /* Check the legal alternatives */
    if (dy == 0) {
	ny = &dy;
	nx = &dx;
    }
    else {
	ny = &dx;
	nx = &dy;
    }

    for (*nx = 0, *ny = -1; *ny < 2; *ny += 2) {
	/* New position */
	nh.y = hero.y + dy;
	nh.x = hero.x + dx;

	if (nh.x < 0 || nh.x > ur_cols - 1 || nh.y < 1 || nh.y > ur_lines - 3)
	    continue;
	ch = winat(nh.y, nh.x);
	switch (ch) {
	    case ' ':
	    case '|':
	    case '-':
		break;
	    default:
		legal++;
		y = dy;
		x = dx;
	}
    }

    /* If we have 2 legal moves, make no change */
    if (legal != 1)
	return;

    /* Make the change */
    if (y == 0) {       /* Move horizontally */
	if (x == 1)
	    runch = 'l';
	else
	    runch = 'h';
    }
    else {          /* Move vertically */
	if (y == 1)
	    runch = 'j';
	else
	    runch = 'k';
    }
    return;
}


/*
 * do_move: Check to see that a move is legal.  If it is handle the
 * consequences (fighting, picking up, etc.)
 */

void
do_move(int dy, int dx)
{
    char    ch;
    coord   old_hero;
    char    hch;
    struct thing *pp = &player;

    firstmove = FALSE;
    if (player.t_no_move) {
	player.t_no_move--;
	msg("You are still stuck in the bear trap.");
	return;
    }

    /*
     * Do a confused move (maybe)
     */
    if ((rnd(100) < 80 && on(player, ISHUH)) ||
	(is_wearing(R_DELUSION) && rnd(100) < 25) ||
	on(player, STUMBLER) && rnd(40) == 0)
	nh = *rndmove(&player);
    else {
	nh.y = hero.y + dy;
	nh.x = hero.x + dx;
    }

    /*
     * Check if he tried to move off the screen or make an illegal
     * diagonal move, and stop him if he did.
     */
    if (nh.x < 0 || nh.x > ur_cols - 1 || nh.y < 1 || nh.y >= ur_lines - 2
	|| !diag_ok(&hero, &nh, &player)) {
	after = fighting = running = FALSE;
	return;
    }
    if (running && ce(hero, nh))
	after = running = FALSE;
    ch = winat(nh.y, nh.x);

    /* Take care of hero trying to move close to something frightening */
    if (on(player, ISFLEE)) {
	if (rnd(10) < 1) {
	    turn_off(player, ISFLEE);
	    msg("You regain your composure.");
	}
	else if (DISTANCE(nh.y, nh.x, player.t_chasee->t_pos.y,
	    player.t_chasee->t_pos.x) <
	DISTANCE(hero.y, hero.x, player.t_chasee->t_pos.y,
	    player.t_chasee->t_pos.x))
	    return;
    }

    /* Take care of hero being held */
    if (on(player, ISHELD) && !isalpha(ch)) {
	if (rnd(pstats.s_str) > 14) {
	    msg("You break free of the hold.");
	    if (--hold_count == 0)
		turn_off(player, ISHELD);
	}
	else {
	    msg("You are being held.");
	    return;
	}
    }

    if (on(player, ISDISGUISE) && rnd(11 * pstats.s_dext) == 0) {
	extinguish_fuse(FUSE_UNDISGUISE);
	undisguise(NULL);
    }

    /* assume he's not in a wall */
    if (!isalpha(ch))
	turn_off(player, ISINWALL);

    hch = mvinch(hero.y, hero.x);   /* Where hero was */
    old_hero = hero;    /* Save hero's old position */

    switch (ch) {
	case ' ':
	case '|':
	case '-':
	case SECRETDOOR:
	    if (off(player, CANINWALL)) {
		after = running = FALSE;
		return;
	    }
	    else if (running) {
		after = running = FALSE;
		return;
	    }
	    turn_on(player, ISINWALL);
	    break;
	case TRAPDOOR:
	case TELTRAP:
	case BEARTRAP:
	case SLEEPTRAP:
	case ARROWTRAP:
	case DARTTRAP:
	case POOL:
	case MAZETRAP:
	case FIRETRAP:
	case POISONTRAP:
	case LAIR:
	case RUSTTRAP:
	    ch = be_trapped(&pp, &nh);
	    if (!is_wearing(R_LEVITATION) && off(player, CANFLY) &&
		(old_hero.x != hero.x || old_hero.y != hero.y
		 || pool_teleport)) {
		pool_teleport = FALSE;
		return;
	    }
	    break;
	case GOLD:
	case POTION:
	case SCROLL:
	case FOOD:
	case WEAPON:
	case ARMOR:
	case RING:
	case ARTIFACT:
	case STICK:
	    running = FALSE;
	    take = ch;
	    break;
	default:
	    break;
    }

    if (ch == FIRETRAP)
	light(&hero);

    hero = nh;      /* Move the hero */

    if (roomin(&hero) == NULL && (hch == '-' || hch == '|' ||
	hch == DOOR || hch == SECRETDOOR)) {
	/* Leaving a room -- darken it */
	struct room *rp = roomin(&old_hero);
	bool    is_lit = FALSE;

	if (!(rp->r_flags & ISDARK))
	    is_lit = TRUE;
	rp->r_flags |= ISDARK;  /* Fake darkness */
	light(&old_hero);
	if (is_lit)
	    rp->r_flags &= ~ISDARK; /* Restore light state */
    }
    else if (ch == DOOR || ch == SECRETDOOR || ch == '|' || ch == '-') {
	/* Entering a room */
	running = FALSE;
	if (hch != '|' && hch != '-')
	    light(&hero);   /* knows whether the hero can see
		     * things in */
    }
    else if (ch == STAIRS)
	running = FALSE;
    else if (ch == POST) {
	running = FALSE;
	new_level(POSTLEV);
	return;
    }
    else if (isalpha(ch)) {
	struct linked_list  *mp;
	struct thing    *tp;
	int t;

	running = FALSE;

	mp = find_mons(hero.y, hero.x);
	if (mp == NULL)
	    return;
	tp = THINGPTR(mp);
	if (good_monster(*tp)) {    /* Exchange places with your
			 * buddy */
	    mvwaddch(cw, old_hero.y, old_hero.x, ch);
	    mvwaddch(mw, old_hero.y, old_hero.x, ch);
	    mvwaddch(mw, hero.y, hero.x, ' ');
	    mvwaddch(cw, hero.y, hero.x, tp->t_oldch);

	    (*tp).t_pos.x = old_hero.x; /* Update monster
			     * position variables */
	    (*tp).t_pos.y = old_hero.y;
	    (*tp).t_oldpos.x = old_hero.x;
	    (*tp).t_oldpos.y = old_hero.y;

	    t = (*tp).t_oldch;
	    (*tp).t_oldch = player.t_oldch;
	    player.t_oldch = t;

	    turn_on(*tp, ISRUN);

	    mvwaddch(cw, hero.y, hero.x, PLAYER);
	    wrefresh(cw);
	    return;
	}
	else {
	    hero = old_hero;    /* Restore hero -- we'll
			 * fight instead of move */
	    fight(&nh, cur_weapon, NOTHROWN);
	    return;
	}
    }
    else
	fighting = FALSE;

    ch = winat(old_hero.y, old_hero.x);
    wmove(cw, old_hero.y, old_hero.x);
    waddch(cw, ch);
    wmove(cw, hero.y, hero.x);
    waddch(cw, PLAYER);
}

/*
 * Called to illuminate a room. If it is dark, remove anything that might
 * move.
 */

void
light(coord *cp)
{
    struct room *rp;
    int j, k, x, y;
    char    ch, rch;
    struct linked_list  *item;
    int jlow, jhigh, klow, khigh;   /* Boundaries of lit area */

    if ((rp = roomin(cp)) != NULL && !on(player, ISBLIND)) {

	/*
	 * is he wearing ring of illumination and in same room?
	 */
	if ((is_wearing(R_LIGHT) || on(player, ISELECTRIC)) &&
	    cp == &hero)
	    rp->r_flags &= ~ISDARK;

	/* If we are in a maze, don't look at the whole room (level) */
	if (levtype == MAZELEV) {
	    jlow = max(0, hero.y - 2 - rp->r_pos.y);
	    jhigh = min(rp->r_max.y, hero.y + 2 - rp->r_pos.y + 1);
	    klow = max(0, hero.x - 2 - rp->r_pos.x);
	    khigh = min(rp->r_max.x, hero.x + 2 - rp->r_pos.x + 1);
	}
	else {
	    jlow = klow = 0;
	    jhigh = rp->r_max.y;
	    khigh = rp->r_max.x;
	}
	for (j = 0; j < rp->r_max.y; j++) {
	    for (k = 0; k < rp->r_max.x; k++) {

		/*
		 * Is this in the give area -- needed for
		 * maze
		 */
		if ((j < jlow || j >= jhigh) &&
		    (k < klow || k >= khigh))
		    continue;

		y = rp->r_pos.y + j;
		x = rp->r_pos.x + k;

		ch = show(y, x);
		wmove(cw, y, x);

		/*
		 * Figure out how to display a secret door
		 */
		if (ch == SECRETDOOR) {
		    if (j == 0 || j == rp->r_max.y - 1)
			ch = '-';
		    else
			ch = '|';
		}

		/*
		 * For monsters, if they were previously not
		 * seen and now can be seen, or vice-versa,
		 * make sure that will happen.
		 */
		if (isalpha(ch)) {
		    struct thing    *tp;

		    item = wake_monster(y, x);
		    if (item == NULL)
			continue;
		    tp = THINGPTR(item);

		    /*
		     * Previously not seen -- now can see
		     * it
		     */
		    if (tp->t_oldch == ' ' &&
			cansee(tp->t_pos.y,
			    tp->t_pos.x))
			tp->t_oldch = mvinch(y, x);

		    /*
		     * Previously seen -- now can't see
		     * it
		     */
		    else if (off(player, ISBLIND) &&
			tp->t_oldch != ' ' &&
			 !cansee(tp->t_pos.y,
			tp->t_pos.x))
			tp->t_oldch = ' ';
		}

		/*
		 * If the room is a dark room, we might want
		 * to remove monsters and the like from it
		 * (since they might move). A dark room or
		 * not in line-of-sight in a maze.
		 */
		if (((rp->r_flags & ISDARK) &&
		    !(rp->r_flags & HASFIRE)) ||
		    (levtype == MAZELEV &&
		     !maze_view(y, x))) {
		    rch = mvwinch(cw, y, x);
		    switch (rch) {
			when    DOOR:
			case STAIRS:
			case TRAPDOOR:
			case TELTRAP:
			case BEARTRAP:
			case SLEEPTRAP:
			case ARROWTRAP:
			case DARTTRAP:
			case POOL:
			case MAZETRAP:
			case FIRETRAP:
			case POISONTRAP:
			case LAIR:
			case RUSTTRAP:
			case POST:
			case '|':
			case '-':
			case ' ':
			    ch = rch;
			when    FLOOR:
			    ch = (on(player,
				ISBLIND) ?
				FLOOR : ' ');
			otherwise:
			    ch = ' ';
		    }
		}
		mvwaddch(cw, y, x, ch);
	    }
	}
    }
}

/*
 * blue_light: magically light up a room (or level or make it dark)
 */

bool
blue_light(int flags)
{
    struct room *rp;
    bool    blessed = (flags & ISBLESSED);
    bool    cursed = (flags & ISCURSED);
    bool    ret_val = FALSE;/* Whether or not affect is known */

    rp = roomin(&hero); /* What room is hero in? */

    /* Darken the room if the magic is cursed */
    if (cursed) {
	if ((rp == NULL) || (rp->r_flags & ISDARK))
	    nothing_message(flags);
	else {
	    if (!(rp->r_flags & HASFIRE))
		msg("The room suddenly goes dark.");
	    else
		nothing_message(flags);
	    rp->r_flags |= ISDARK;
	    ret_val = TRUE;
	}
    }
    else {
	ret_val = TRUE;
	if (rp && (rp->r_flags & ISDARK) && !(rp->r_flags & HASFIRE)) {
	    addmsg("The room is lit");
	    if (!terse)
		addmsg(" by a %s blue light",
		       blessed ? "bright" : "shimmering");
	    addmsg(".");
	    endmsg();
	}
	else if (winat(hero.y, hero.x) == PASSAGE)
	    msg("The corridor glows %sand then fades.",
		blessed ? "brightly " : "");
	else {
	    ret_val = FALSE;
	    nothing_message(flags);
	}
	if (blessed) {
	    short   i;  /* Index through rooms */

	    for (i = 0; i < MAXROOMS; i++)
		rooms[i].r_flags &= ~ISDARK;
	}
	else if (rp)
	    rp->r_flags &= ~ISDARK;
    }

    /*
     * Light the room and put the player back up
     */
    light(&hero);
    mvwaddch(cw, hero.y, hero.x, PLAYER);
    return (ret_val);
}

/*
 * show: returns what a certain thing will display as to the un-initiated
 */

char
show(int y, int x)
{
    char    ch = winat(y, x);
    struct linked_list  *it;
    struct thing    *tp;

    if (isatrap(ch)) {
	struct trap *trp = trap_at(y, x);

	return (trp->tr_flags & ISFOUND) ? ch : trp->tr_show;
    }
    else if (isalpha(ch)) {
	if ((it = find_mons(y, x)) == NULL) {
	    debug("Can't find monster in move.");
	    return ' ';
	}
	tp = THINGPTR(it);

	if (on(*tp, ISDISGUISE))
	    ch = tp->t_disguise;    /* As a mimic */
	else if (on(*tp, ISINVIS) || (on(*tp, ISSHADOW) &&
	    rnd(100) < 90) || on(*tp, CANSURPRISE)) {
	    if (off(player, CANSEE) || on(*tp, CANSURPRISE))
		ch = mvwinch(stdscr, y, x); /* Invisible */
	}
	else if (on(*tp, CANINWALL)) {
	    char    tch;

	    tch = mvwinch(stdscr, y, x);
	    if (tch == WALL || tch == '-' || tch == '|')
		ch = winch(stdscr); /* As Xorn */
	}
    }
    return ch;
}

/*
 * be_trapped: The guy stepped on a trap.... Make him pay.
 */

char
be_trapped(struct thing **th_p, coord *tc)
{
    struct thing *th = *th_p;
    struct trap *tp;
    char    ch, *mname;
    bool    is_player = (th == &player), can_see = cansee(tc->y, tc->x);
    struct linked_list  *mitem;

    tp = trap_at(tc->y, tc->x);
    ch = tp->tr_type;

    if (!is_player) {
	mitem = find_mons(th->t_pos.y, th->t_pos.x);
	mname = monsters[th->t_index].m_name;
	/* Flying monsters do not set off traps */
	if (!mitem || (on(*th, CANFLY) &&
	     (ch == BEARTRAP || ch == MAZETRAP || ch == TRAPDOOR
	      || ch == ARROWTRAP || ch == DARTTRAP))) {
	    debug("%s avoided trap.", mname);
	    return (ch);
	}
    }
    else {
	short   thief_bonus = -50;

	count = running = FALSE;
	mvwaddch(cw, tp->tr_pos.y, tp->tr_pos.x, tp->tr_type);
	if (no_command)
	    return (ch);
	if (player.t_ctype == C_THIEF || player.t_ctype == C_NINJA)
	    thief_bonus = 10;
	if (((is_wearing(R_LEVITATION) || on(player, CANFLY)) &&
	    (ch != FIRETRAP ||
		 (ch == FIRETRAP && !(tp->tr_flags & ISFOUND))))
	    || (moving && (tp->tr_flags & ISFOUND) && rnd(100) <
	      thief_bonus + 2 * pstats.s_dext + 5 * pstats.s_lvl) &&
	    (ch == BEARTRAP || ch == MAZETRAP || ch == TRAPDOOR
	     || ch == ARROWTRAP || ch == DARTTRAP)) {
	    msg(tr_name(ch));
	    tp->tr_flags |= ISFOUND;
	    return (ch);
	}
	if (moving)
	    msg("Your attempt fails.");
    }
    tp->tr_flags |= ISFOUND;
    switch (ch) {
	when    TRAPDOOR:
	    if (is_player) {
		level++;
		new_level(NORMLEV);
		addmsg("You fell into a trap");
		if (player.t_ctype != C_THIEF
		    && player.t_ctype != C_ASSASIN
		    && player.t_ctype != C_NINJA
		    && rnd(pstats.s_dext) < 4) {
		    addmsg(" and were damaged by the fall");
		    if ((pstats.s_hpt -= roll(1, 6)) <= 0) {
			addmsg("!  The fall killed you.");
			endmsg();
			death(D_FALL);
			return (ch);
		    }
		}
		addmsg("!");
		endmsg();
		if (off(player, ISCLEAR) && rnd(4) < 3) {
		    if (on(player, ISHUH))
			lengthen_fuse(FUSE_UNCONFUSE, rnd(8) +
			    HUHDURATION);
		    else
			light_fuse(FUSE_UNCONFUSE, 0, rnd(8) +
			    HUHDURATION, AFTER);
		    turn_on(player, ISHUH);
		}
		else
		    msg("You feel dizzy for a moment, but it quickly passes.");
	    }
	    else {
		if (can_see)
		    msg("The %s fell into a trap!", mname);
		check_residue(th);
		remove_monster(&th->t_pos, mitem);
		*th_p = NULL;
		return ch;
	    }
	when    BEARTRAP:
	    if (is_stealth(th)) {
		if (is_player)
		    msg("You pass a bear trap.");
		else if (can_see)
		    msg("The %s passes a bear trap.",
		    mname);
	    }
	    else {
		th->t_no_move += BEARTIME;
		if (is_player)
		    msg("You are caught in a bear trap.");
		else if (can_see)
		    msg("The %s is caught in a bear trap.",
			mname);
	    }
	when    SLEEPTRAP:
	    if (is_player) {
		msg("A strange white mist envelops you.");
		if (!is_wearing(R_ALERT)) {
		    if (!is_wearing(R_BREATHE) &&
			off(player, HASOXYGEN)) {
			msg("You fall asleep.");
			no_command += SLEEPTIME;
		    }
		}
	    }
	    else {
		if (can_see)
		    msg("A strange white mist envelops the %s.", mname);
		if (on(*th, ISUNDEAD)) {
		    if (can_see)
			msg("The mist doesn't seem to affect the %s.", mname);
		}
		if (on(*th, ISUNDEAD) || on(*th, HASOXYGEN)) {
		    if (can_see)
			msg("The mist doesn't seem to affect the %s.", mname);
		}
		else {
		    th->t_no_move += SLEEPTIME;
		}
	    }
	when    ARROWTRAP:
	    if (swing(th->t_ctype, th->t_stats.s_lvl - 1,
		th->t_stats.s_arm, 1)) {
		if (is_player) {
		    msg("Oh no! An arrow shot you.");
		    if ((pstats.s_hpt -= roll(1, 6)) <= 0) {
			msg("The arrow killed you.");
			death(D_ARROW);
			return ch;
		    }
		}
		else {
		    if (can_see)
			msg("An arrow shot the %s.", mname);
		    if (on(*th, NOSHARP)) {
			if (can_see)
			    msg("The arrow has no effect!");
		    }
		    else {
			if ((th->t_stats.s_hpt -= roll(1, 6)) <= 0) {
			    if (can_see)
				msg("The arrow killed the %s.", mname);
			    killed(NULL, mitem,
				NOMESSAGE,
				NOPOINTS);
			}
		    }
		}
	    }
	    else {
		struct linked_list  *item;
		struct object   *arrow;

		if (is_player)
		    msg("An arrow shoots past you.");
		else if (can_see)
		    msg("An arrow shoots by the %s.",
			mname);
		item = new_item(sizeof *arrow);
		arrow = OBJPTR(item);
		arrow->o_type = WEAPON;
		arrow->o_which = ARROW;
		arrow->o_hplus = rnd(3) - 1;
		arrow->o_dplus = rnd(3) - 1;
		init_weapon(arrow, ARROW);
		arrow->o_count = 1;
		arrow->o_pos = *tc;
		arrow->o_mark[0] = '\0';
		fall(&player, item, FALSE);
	    }
	when    TELTRAP:
	    if (is_player) {
		teleport();
		if (off(player, ISCLEAR)) {
		    msg("Wait, what's going on here. Huh? What? Who?");
		    if (on(player, ISHUH))
			lengthen_fuse(FUSE_UNCONFUSE, rnd(8) +
			    HUHDURATION);
		    else
			light_fuse(FUSE_UNCONFUSE, 0, rnd(8) +
			    HUHDURATION, AFTER);
		    turn_on(player, ISHUH);
		}
		else
		    msg("You feel dizzy for a moment, but it quickly passes.");
	    }
	    else {
		int rm;

		/* Erase the monster from the old position */
		if (isalpha(mvwinch(cw, th->t_pos.y,
		    th->t_pos.x)))
		    mvwaddch(cw, th->t_pos.y, th->t_pos.x,
			th->t_oldch);
		mvwaddch(mw, th->t_pos.y, th->t_pos.x, ' ');

		/* Get a new position */
		do {
		    rm = rnd_room();
		    rnd_pos(&rooms[rm], &th->t_pos);
		} until(winat(th->t_pos.y, th->t_pos.x)
		    == FLOOR);

		/* Put it there */
		mvwaddch(mw, th->t_pos.y, th->t_pos.x,
		    th->t_type);
		th->t_oldch = mvwinch(cw, th->t_pos.y,
		    th->t_pos.x);
		if (can_see)
		    msg("The %s seems to have disappeared!", mname);
	    }
	when    DARTTRAP:
	    if (swing(th->t_ctype, th->t_stats.s_lvl + 1,
		th->t_stats.s_arm, 1)) {
		if (is_player) {
		    msg("A small dart just hit you in the shoulder.");
		    if ((pstats.s_hpt -= roll(1, 4)) <= 0) {
			msg("The dart killed you.");
			death(D_DART);
			return (ch);
		    }

		    /* Now the poison */
		    if (player.t_ctype != C_PALADIN
			&& !(player.t_ctype == C_NINJA &&
			pstats.s_lvl > 12)
			&& !save(VS_POISON)) {

			/*
			 * 75% chance it will do point
			 * damage - else strength
			 */
			if (rnd(100) < 75) {
			    pstats.s_hpt /= 2;
			    if (pstats.s_hpt == 0) {
				death(D_POISON);
				return (ch);
			    }
			}
			else if (!is_wearing(R_SUSABILITY))
			    chg_str(-1, FALSE, FALSE);
		    }
		}
		else {
		    int orig_hp = th->t_stats.s_hpt;

		    if (can_see)
			msg("A small dart just hit the %s.", mname);

		    /*
		     * Poison has no effect on poisonous or
		     * undead monsters
		     */
		    if (off(*th, CANPOISON) &&
			off(*th, ISUNDEAD) &&
			!save_throw(VS_POISON, th))
			th->t_stats.s_hpt /= 2;

		    /* Now the dart damage */
		    if (off(*th, NOSHARP))
			th->t_stats.s_hpt -= roll(1, 4);

		    if (orig_hp == th->t_stats.s_hpt)
			if (can_see)
			    msg("The dart has not effect!");
			else if (th->t_stats.s_hpt < 0) {
			    if (can_see)
				msg("The dart killed the %s.", mname);
			    killed(NULL, mitem, NOMESSAGE, NOPOINTS);
			}
		}
	    }
	    else {
		if (is_player)
		    msg("A small dart whizzes by your ear and vanishes.");
		else if (can_see)
		    msg("A small dart whizzes by the %s and vanishes.",
			mname);
	    }
	when    POOL:{
	    int i;

	    i = rnd(100);
	    if (is_player) {
		if (on(player, ISELECTRIC)) {
		    msg("Oh no!!! The water shorts you out");
		    extinguish_fuse(FUSE_UNELECTRIFY);
		    turn_off(player, ISELECTRIC);
		    if (!is_wearing(R_ELECTRESIST)) {
			if ((pstats.s_hpt -= roll(1, 10)) <= 0) {
			    addmsg("!  The shock killed you.");
			    endmsg();
			    death(D_DROWN);
			    return (ch);
			}
		    }
		}
		if ((tp->tr_flags & ISGONE)) {
		    if (i < 30) {
			teleport(); /* teleport away */
			if (off(player, ISCLEAR)) {
			    if (on(player, ISHUH))
				lengthen_fuse(FUSE_UNCONFUSE, rnd(8) + HUHDURATION);
			    else
				light_fuse(FUSE_UNCONFUSE, 0, rnd(8) + HUHDURATION, AFTER);
			    turn_on(player, ISHUH);
			}
			else
			    msg("You feel dizzy for a moment, but it quickly passes.");
			pool_teleport = TRUE;
		    }
		    else if ((i < 45) && level > 2) {
			level -= rnd(2) + 1;
			new_level(NORMLEV);
			pool_teleport = TRUE;
			msg("You here a faint groan from below.");
			if (off(player, ISCLEAR)) {
			    if (on(player, ISHUH))
				lengthen_fuse(FUSE_UNCONFUSE, rnd(8) + HUHDURATION);
			    else
				light_fuse(FUSE_UNCONFUSE, 0, rnd(8) + HUHDURATION, AFTER);
			    turn_on(player, ISHUH);
			}
			else
			    msg("You feel dizzy for a moment, but it quickly passes.");
		    }
		    else if (i < 70) {
			level += rnd(4) + 1;
			new_level(NORMLEV);
			pool_teleport = TRUE;
			msg("You find yourself in strange surroundings.");
			if (off(player, ISCLEAR)) {
			    if (on(player, ISHUH))
				lengthen_fuse(FUSE_UNCONFUSE, rnd(8) + HUHDURATION);
			    else
				light_fuse(FUSE_UNCONFUSE, 0, rnd(8) + HUHDURATION, AFTER);
			    turn_on(player, ISHUH);
			}
			else
			    msg("You feel dizzy for a moment, but it quickly passes.");
		    }
		    else if (i > 95) {
			if (is_wearing(R_BREATHE) || on(player, HASOXYGEN))
			    msg("You splash in the pool unharmed.");
			else {
			    msg("Oh no!!! You drown in the pool!!! --More--");
			    wait_for(' ');
			    death(D_DROWN);
			    return (ch);
			}
		    }
		}
	    }
	    else {
		if (can_see)
		    msg("The %s fell into the pool!", mname);
		if (i < 30) {
		    if (off(*th, HASOXYGEN)) {
			if (can_see)
			    msg("The %s has drowned!", mname);
			killed(NULL, mitem, NOMESSAGE, NOPOINTS);
		    }
		}
	    }
	}
	when    MAZETRAP:
	    if (is_player) {
		level++;
		new_level(MAZELEV);
		addmsg("You are surrounded by twisty passages");
		if (rnd(4) < 1) {
		    addmsg(" and were damaged by the fall");
		    if ((pstats.s_hpt -= roll(1, 6)) <= 0) {
			addmsg("!  The fall killed you.");
			endmsg();
			death(D_FALL);
			return (ch);
		    }
		}
		addmsg("!");
		endmsg();
		if (off(player, ISCLEAR)) {
		    if (on(player, ISHUH))
			lengthen_fuse(FUSE_UNCONFUSE, rnd(8) +
			    HUHDURATION);
		    else {
			light_fuse(FUSE_UNCONFUSE, 0, rnd(8) +
			    HUHDURATION, AFTER);
			turn_on(player, ISHUH);
		    }
		}
		else
		    msg("You feel dizzy for a moment, but it quickly passes.");
	    }
	    else {
		if (can_see)
		    msg("The %s fell into a trap!", mname);
		check_residue(th);
		remove_monster(&th->t_pos, mitem);
		*th_p = NULL;
		return ch;
	    }
	when    FIRETRAP:{
	    struct room *rp = roomin(&hero);

	    if (is_player) {
		if (is_wearing(R_FIRERESIST) || on(player, NOFIRE))
		    msg("You pass through the flames unharmed.");
		else {
		    addmsg("You are burned by the flames");
		    if ((pstats.s_hpt -= roll(pstats.s_lvl, 2)) <= 0) {
			addmsg("!  The flames killed you.");
			endmsg();
			death(D_FIRE);
			return (ch);
		    }
		    addmsg("!");

		    endmsg();
		}
	    }
	    else {
		if (on(*th, CANBBURN)) {
		    if (can_see)
			msg("The %s is burned to death by the flames.", mname);
		    check_residue(th);
		    remove_monster(&th->t_pos, mitem);
		    *th_p = NULL;
		    return ch;
		}
		else if (on(*th, NOFIRE)) {
		    if (can_see)
			msg("The %s passes through the flames unharmed.", mname);
		}
		else {
		    if (can_see)
			msg("The %s is burned by the flames.", mname);
		    if ((th->t_stats.s_hpt -= roll(th->t_stats.s_lvl, 3)) < 0) {
			if (can_see)
			    msg("The %s is burned to death by the flames.", mname);
			check_residue(th);
			remove_monster(&th->t_pos, mitem);
			*th_p = NULL;
			return ch;
		    }
		    else if (th->t_stats.s_intel < rnd(20)) {
			if (can_see)
			    msg("The %s turns and runs away in fear.", mname);
			turn_on(*th, ISFLEE);
		    }
		}
	    }
	    if (rp != NULL) {
		rp->r_flags &= ~ISDARK;
		light(&hero);
	    }
	}
	when    POISONTRAP:
	    if (is_player) {
		msg("You fall into a pool of poison.");
		if (rnd(4) > 0) {
		    msg("You swallow some of the liquid and feel very sick.");
		    pstats.s_hpt -= pstats.s_hpt / 3;
		    if (player.t_ctype != C_PALADIN
			&& !(player.t_ctype == C_NINJA &&
			pstats.s_lvl > 12)
			    && !is_wearing(R_SUSABILITY))
			chg_str(-2, FALSE, FALSE);
		}
		else
		msg("The stuff tastes horrible.");
	    }
	    else {
		if (can_see)
		    msg("The %s falls into the pool of poison.", mname);
		if (rnd(4) > 0 && off(*th, ISUNDEAD))
		    th->t_stats.s_hpt *= 2.0 / 3.0;
	    }
	when    LAIR:
	    if (is_player) {
		msg("You found a monster lair!");
		mpos = 0;
		new_level(THRONE);
	    }
	    else {
		if (can_see)
		    msg("The %s fell into a trap!", mname);
		check_residue(th);
		remove_monster(&th->t_pos, mitem);
		*th_p = NULL;
		return ch;
	    }
	when    RUSTTRAP:
	    if (is_player) {
		msg("You are splashed by water.");
		if (cur_armor != NULL &&
		    cur_armor->o_which != SOFT_LEATHER &&
		    cur_armor->o_which != HEAVY_LEATHER &&
		    cur_armor->o_which != CUIRBOLILLI &&
		    cur_armor->o_which != PADDED_ARMOR &&
		    cur_armor->o_which != CRYSTAL_ARMOR &&
		    cur_armor->o_which != MITHRIL &&
		    !(cur_armor->o_flags & ISPROT) &&
		    cur_armor->o_ac < pstats.s_arm + 1) {
		    msg(terse ? "Your armor weakens!"
			: "Your armor appears to be weaker now. Oh my!");
		    cur_armor->o_ac++;
		}
		else if (cur_armor != NULL && (cur_armor->o_flags & ISPROT))
		    msg(terse ? "" : "The rust vanishes instantly!");
	    }
	    else {
		if (can_see)
		    msg("The %s is splashed by water.", mname);
	    }
    }
    return (ch);
}

/*
 * dip_it: Dip an object into a magic pool
 */
void
dip_it()
{
    struct linked_list  *what;
    struct object   *ob;
    struct trap *tp;
    int wh, i;

    tp = trap_at(hero.y, hero.x);
    if (tp == NULL || !(tp->tr_type == POOL || tp->tr_type == POISONTRAP)) {
	msg("I see no pools here.");
	return;
    }
    if (tp->tr_flags & ISGONE) {
	msg("This %s appears to have been used once already.",
	 (tp->tr_type == POOL ? "shimmering pool" : "poison pool"));
	return;
    }
    if ((what = get_item("dip", 0)) == NULL) {
	msg("");
	after = FALSE;
	return;
    }
    ob = OBJPTR(what);
    mpos = 0;
    if (ob == cur_armor) {
	msg("You have to take off your armor before you can dip it.");
	return;
    }
    else if (ob == cur_ring[LEFT_1] || ob == cur_ring[LEFT_2] ||
	 ob == cur_ring[LEFT_3] || ob == cur_ring[LEFT_4] ||
	 ob == cur_ring[RIGHT_1] || ob == cur_ring[RIGHT_2] ||
	 ob == cur_ring[RIGHT_3] || ob == cur_ring[RIGHT_4]) {
	msg("You have to take that ring off before you can dip it.");
	return;
    }
    tp->tr_flags |= ISGONE;
    if (ob != NULL && tp->tr_type == POOL) {
	wh = ob->o_which;
	ob->o_flags |= ISKNOW;
	i = rnd(100);
	switch (ob->o_type) {
	    case WEAPON:
		if (i < 50) {
		    if (!(ob->o_flags & ISCURSED)) {
			ob->o_hplus += 1;
			ob->o_dplus += 1;
		    }
		    else {
			ob->o_hplus = rnd(2);
			ob->o_dplus = rnd(2);
		    }
		    ob->o_flags &= ~ISCURSED;
		    msg("The %s glows blue for a moment.",
			weaps[wh].w_name);
		}
		else if (i < 70) {  /* curse weapon here */
		    if (!(ob->o_flags & ISCURSED)) {
			ob->o_hplus = -(rnd(2) + 1);
			ob->o_dplus = -(rnd(2) + 1);
		    }
		    else {  /* if already cursed */
			ob->o_hplus--;
			ob->o_dplus--;
		    }
		    ob->o_flags |= ISCURSED;
		    msg("The %s glows red for a moment.",
			weaps[wh].w_name);
		}
		else
		    msg("Nothing seems to happen.");
	    when    ARMOR:
		if (i < 50) {   /* enchant armor */
		    if (!(ob->o_flags & ISCURSED))
			ob->o_ac -= rnd(2) + 1;
		    else
			ob->o_ac = -rnd(3) +
			    armors[wh].a_class;
		    ob->o_flags &= ~ISCURSED;
		    msg("The %s glows blue for a moment.",
			armors[wh].a_name);
		}
		else if (i < 75) {  /* curse armor */
		    if (!(ob->o_flags & ISCURSED))
			ob->o_ac = rnd(3) +
			    armors[wh].a_class;
		    else
			ob->o_ac += rnd(2) + 1;
		    ob->o_flags |= ISCURSED;
		    msg("The %s glows red for a moment.",
			armors[wh].a_name);
		}
		else
		    msg("Nothing seems to happen");
	    when    STICK:{
		int j;
		j = rnd(8) + 1;
		if (i < 50) {   /* add charges */
		    ob->o_charges += j;
		    know_items[TYP_STICK][wh] = TRUE;
		    if (ob->o_flags & ISCURSED)
			ob->o_flags &= ~ISCURSED;
		    msg("The %s %s glows blue for a moment.",
			ws_made[wh], ws_type[wh]);
		}
		else if (i < 65) {  /* remove charges */
		    if ((ob->o_charges -= i) < 0)
			ob->o_charges = 0;
		    know_items[TYP_STICK][wh] = TRUE;
		    if (ob->o_flags & ISBLESSED)
			ob->o_flags &= ~ISBLESSED;
		    else
			ob->o_flags |= ISCURSED;
		    msg("The %s %s glows red for a moment.",
			ws_made[wh], ws_type[wh]);
		}
		else
		    msg("Nothing seems to happen.");
	    }
	    when    SCROLL:
		know_items[TYP_SCROLL][wh] = TRUE;
		msg("The '%s' scroll unfurls.", s_names[wh]);
	    when    POTION:
		know_items[TYP_POTION][wh] = TRUE;
		msg("The %s potion bubbles for a moment.", p_colors[wh]);
	    when    RING:
		if (i < 50) {   /* enchant ring */
		    if (!(ob->o_flags & ISCURSED))
			ob->o_ac += rnd(2) + 1;
		    else
			ob->o_ac = rnd(2) + 1;
		    ob->o_flags &= ~ISCURSED;
		}
		else if (i < 80) {  /* curse ring */
		    if (!(ob->o_flags & ISCURSED))
			ob->o_ac = -(rnd(2) + 1);
		    else
			ob->o_ac -= (rnd(2) + 1);
		    ob->o_flags |= ISCURSED;
		}
		know_items[TYP_RING][wh] = TRUE;
		msg("The %s ring vibrates for a moment.",
		    r_stones[wh]);
	    otherwise:
		msg("The pool bubbles for a moment.");
	}
    }
    else if (ob != NULL && tp->tr_type == POISONTRAP) {
	if ((player.t_ctype == C_PALADIN) ||
	    (player.t_ctype == C_CLERIC && rnd(2))) {
	    msg("Trying to use poison is evil.");
	    luck += 2;
	}
	if (ob->o_type != WEAPON || rnd(10) > 0)
	    msg("Nothing seems to happen.");
	else {
	    msg("Your %s is covered with a black sticky liquid.",
		weaps[ob->o_which].w_name);
	    ob->o_flags |= ISPOISON;
	}
    }
    else
	msg("Nothing seems to happen.");
}

/*
 * trap_at: find the trap at (y,x) on screen.
 */

struct trap *
trap_at(int y, int x)
{
    struct trap *tp, *ep;

    ep = &traps[ntraps];
    for (tp = traps; tp < ep; tp++)
	if (tp->tr_pos.y == y && tp->tr_pos.x == x)
	    break;
    if (tp == ep) {
	debug((sprintf(prbuf, "Trap at %d,%d not in array", y, x),
	    prbuf));
	tp = NULL;
    }
    return tp;
}

/*
 * set_trap: set a trap at (y, x) on screen.
 */

void
set_trap(struct thing *tp, int y, int x)
{
    bool    is_player = (tp == &player);
    int selection = rnd(7) + 1;
    char    ch, och;
    bool    thief_bonus = 0;

    switch (och = mvinch(y, x)) {
	case WALL:
	case FLOOR:
	case PASSAGE:
	    break;
	default:
	    msg("The trap failed!");
	    return;
    }

    if (is_player && (player.t_ctype == C_THIEF ||
	player.t_ctype == C_NINJA))
	thief_bonus = 10;

    if (ntraps >= 2 * MAXTRAPS || ++trap_tries >= MAXTRPTRY ||
	rnd(60) >= (tp->t_stats.s_dext + tp->t_stats.s_lvl / 2 +
	thief_bonus)) {
	if (is_player)
	    msg("The trap failed!");
	return;
    }

    /* Set up for redraw */
    clearok(cw, TRUE);
    touchwin(cw);

    if (is_player) {
	add_line("[1] Trap Door");
	add_line("[2] Bear Trap");
	add_line("[3] Sleep Trap");
	add_line("[4] Arrow Trap");
	add_line("[5] Teleport Trap");
	add_line("[6] Dart Trap");
	add_line("[7] Fire Trap");
	end_line();
	msg("Which trap? ");

	selection = (short) ((getchar() & 0177) - '0');
	while (selection < 1 || selection > 7) {
	    if (selection == (short) ESCAPE - (short) '0') {
		after = FALSE;
		return;
	    }
	    msg("");
	    msg("Please enter a selection between 1 and 7:  ");
	    selection = (short) ((getchar() & 0177) - '0');
	}
    }

    switch (selection) {
	when 1: ch = TRAPDOOR;
	when 2: ch = BEARTRAP;
	when 3: ch = SLEEPTRAP;
	when 4: ch = ARROWTRAP;
	when 5: ch = TELTRAP;
	when 6: ch = DARTTRAP;
	when 7: ch = FIRETRAP;
    }

    mvaddch(y, x, ch);
    traps[ntraps].tr_type = ch;
    traps[ntraps].tr_flags = ISTHIEFSET;
    traps[ntraps].tr_show = och;
    traps[ntraps].tr_pos.y = y;
    traps[ntraps++].tr_pos.x = x;
}


/*
 * rndmove: move in a random direction if the monster/person is confused
 */

coord   *
rndmove(struct thing *who)
{
    int x, y;
    int ex, ey, nopen = 0;
    static coord    ret;    /* what we will be returning */
    static coord    dest;

    ret = who->t_pos;

    /*
     * Now go through the spaces surrounding the player and set that
     * place in the array to true if the space can be moved into
     */
    ey = ret.y + 1;
    ex = ret.x + 1;
    for (y = who->t_pos.y - 1; y <= ey; y++)
	if (y > 0 && y < ur_lines - 2)
	    for (x = who->t_pos.x - 1; x <= ex; x++) {
		if (x < 0 || x >= ur_cols)
		    continue;
		if (step_ok(y, x, NOMONST, who)) {
		    dest.y = y;
		    dest.x = x;
		    if (!diag_ok(&who->t_pos, &dest, who))
			continue;
		    if (rnd(++nopen) == 0)
			ret = dest;
		}
	    }
    return &ret;
}

/*
 * isatrap: Returns TRUE if this character is some kind of trap
 */
bool
isatrap(char ch)
{
    switch (ch) {
	case DARTTRAP:
	case TELTRAP:
	case TRAPDOOR:
	case ARROWTRAP:
	case SLEEPTRAP:
	case POOL:
	case MAZETRAP:
	case FIRETRAP:
	case POISONTRAP:
	case LAIR:
	case RUSTTRAP:
	case BEARTRAP:
	    return (TRUE);
	default:
	    return (FALSE);
    }
}

/*
    step_ok()
        returns true if it is ok for type to step on ch flgptr will be
        NULL if we don't know what the monster is yet!
*/

int
step_ok(int y, int x, int can_on_monst, struct thing *flgptr)
{
    struct linked_list *item;
    char ch;

    /* What is here?  Don't check monster window if MONSTOK is set */

    if (can_on_monst == MONSTOK)
        ch = CCHAR( mvinch(y, x) );
    else
        ch = winat(y, x);

    switch (ch)
    {
        case ' ':
        case '|':
        case '-':
        case SECRETDOOR:
            if (flgptr && on(*flgptr, CANINWALL))
                return(TRUE);

            return(FALSE);

        case SCROLL:
            /*
             * If it is a scroll, it might be a scare monster scroll so
             * we need to look it up to see what type it is.
             */

            if (flgptr && flgptr->t_ctype == C_MONSTER)
            {
                item = find_obj(y, x, TRUE);

                if (item != NULL && (OBJPTR(item))->o_type == SCROLL
                     && (OBJPTR(item))->o_which == S_SCARE
                     && rnd(flgptr->t_stats.s_intel) < 12)
                return(FALSE); /* All but smart ones are scared */
            }
            return(TRUE);

        default:
            return(!isalpha(ch));
    }
}

