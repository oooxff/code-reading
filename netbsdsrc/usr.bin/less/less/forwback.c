/*	$NetBSD: forwback.c,v 1.3 1997/09/21 12:40:59 mrg Exp $	*/

/*
 * Copyright (c) 1984,1985,1989,1994,1995,1996  Mark Nudelman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice in the documentation and/or other materials provided with 
 *    the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN 
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/*
 * Primitives for displaying the file on the screen,
 * scrolling either forward or backward.
 */

#include "less.h"
#include "position.h"

public int hit_eof;	/* Keeps track of how many times we hit end of file */
public int screen_trashed;
public int squished;
public int no_back_scroll = 0;

extern int sigs;
extern int top_scroll;
extern int quiet;
extern int sc_width, sc_height;
extern int quit_at_eof;
extern int plusoption;
extern int forw_scroll;
extern int back_scroll;
extern int ignore_eoi;
#if TAGS
extern char *tagoption;
#endif

/*
 * Sound the bell to indicate user is trying to move past end of file.
 */
	static void
eof_bell()
{
	if (quiet == NOT_QUIET)
		bell();
	else
		vbell();
}

/*
 * Check to see if the end of file is currently "displayed".
 */
	static void
eof_check()
{
	POSITION pos;

	if (ignore_eoi)
		return;
	if (ABORT_SIGS())
		return;
	/*
	 * If the bottom line is empty, we are at EOF.
	 * If the bottom line ends at the file length,
	 * we must be just at EOF.
	 */
	pos = position(BOTTOM_PLUS_ONE);
	if (pos == NULL_POSITION || pos == ch_length())
		hit_eof++;
}

/*
 * If the screen is "squished", repaint it.
 * "Squished" means the first displayed line is not at the top
 * of the screen; this can happen when we display a short file
 * for the first time.
 */
	static void
squish_check()
{
	if (!squished)
		return;
	squished = 0;
	repaint();
}

/*
 * Display n lines, scrolling forward, 
 * starting at position pos in the input file.
 * "force" means display the n lines even if we hit end of file.
 * "only_last" means display only the last screenful if n > screen size.
 * "nblank" is the number of blank lines to draw before the first
 *   real line.  If nblank > 0, the pos must be NULL_POSITION.
 *   The first real line after the blanks will start at ch_zero().
 */
	public void
forw(n, pos, force, only_last, nblank)
	register int n;
	POSITION pos;
	int force;
	int only_last;
	int nblank;
{
	int eof = 0;
	int nlines = 0;
	int do_repaint;
	static int first_time = 1;

	squish_check();

	/*
	 * do_repaint tells us not to display anything till the end, 
	 * then just repaint the entire screen.
	 * We repaint if we are supposed to display only the last 
	 * screenful and the request is for more than a screenful.
	 * Also if the request exceeds the forward scroll limit
	 * (but not if the request is for exactly a screenful, since
	 * repainting itself involves scrolling forward a screenful).
	 */
	do_repaint = (only_last && n > sc_height-1) || 
		(forw_scroll >= 0 && n > forw_scroll && n != sc_height-1);

	if (!do_repaint)
	{
		if (top_scroll && n >= sc_height - 1 && pos != ch_length())
		{
			/*
			 * Start a new screen.
			 * {{ This is not really desirable if we happen
			 *    to hit eof in the middle of this screen,
			 *    but we don't yet know if that will happen. }}
			 */
			pos_clear();
			add_forw_pos(pos);
			force = 1;
			if (top_scroll == OPT_ONPLUS || first_time)
				clear();
			home();
		} else
		{
			clear_bot();
			/*
			 * Remove the top n lines and scroll the rest
			 * upward, leaving cursor at first new blank line.
			 */
			remove_top(n);
		}

		if (pos != position(BOTTOM_PLUS_ONE) || empty_screen())
		{
			/*
			 * This is not contiguous with what is
			 * currently displayed.  Clear the screen image 
			 * (position table) and start a new screen.
			 */
			pos_clear();
			add_forw_pos(pos);
			force = 1;
			if (top_scroll)
			{
				if (top_scroll == OPT_ONPLUS)
					clear();
				home();
			} else if (!first_time)
			{
				putstr("...skipping...\n");
			}
		}
	}

	while (--n >= 0)
	{
		/*
		 * Read the next line of input.
		 */
		if (nblank > 0)
		{
			/*
			 * Still drawing blanks; don't get a line 
			 * from the file yet.
			 * If this is the last blank line, get ready to
			 * read a line starting at ch_zero() next time.
			 */
			if (--nblank == 0)
				pos = ch_zero();
		} else
		{
			/* 
			 * Get the next line from the file.
			 */
			pos = forw_line(pos);
			if (pos == NULL_POSITION)
			{
				/*
				 * End of file: stop here unless the top line 
				 * is still empty, or "force" is true.
				 * Even if force is true, stop when the last
				 * line in the file reaches the top of screen.
				 */
				eof = 1;
				if (!force && position(TOP) != NULL_POSITION)
					break;
				if (!empty_lines(0, 0) && 
				    !empty_lines(1, 1) &&
				     empty_lines(2, sc_height-1))
					break;
			}
		}
		/*
		 * Add the position of the next line to the position table.
		 * Display the current line on the screen.
		 */
		add_forw_pos(pos);
		nlines++;
		if (do_repaint)
			continue;
		/*
		 * If this is the first screen displayed and
		 * we hit an early EOF (i.e. before the requested
		 * number of lines), we "squish" the display down
		 * at the bottom of the screen.
		 * But don't do this if a + option or a -t option
		 * was given.  These options can cause us to
		 * start the display after the beginning of the file,
		 * and it is not appropriate to squish in that case.
		 */
		if (first_time && pos == NULL_POSITION && !top_scroll && 
#if TAGS
		    tagoption == NULL &&
#endif
		    !plusoption)
		{
			squished = 1;
			continue;
		}
		if (top_scroll == OPT_ON)
			clear_eol();
		put_line();
	}

	if (ignore_eoi)
		hit_eof = 0;
	else if (eof && !ABORT_SIGS())
		hit_eof++;
	else
		eof_check();
	if (nlines == 0)
		eof_bell();
	else if (do_repaint)
		repaint();
	first_time = 0;
	(void) currline(BOTTOM);
}

/*
 * Display n lines, scrolling backward.
 */
	public void
back(n, pos, force, only_last)
	register int n;
	POSITION pos;
	int force;
	int only_last;
{
	int nlines = 0;
	int do_repaint;

	squish_check();
	do_repaint = (n > get_back_scroll() || (only_last && n > sc_height-1));
	hit_eof = 0;
	while (--n >= 0)
	{
		/*
		 * Get the previous line of input.
		 */
		pos = back_line(pos);
		if (pos == NULL_POSITION)
		{
			/*
			 * Beginning of file: stop here unless "force" is true.
			 */
			if (!force)
				break;
		}
		/*
		 * Add the position of the previous line to the position table.
		 * Display the line on the screen.
		 */
		add_back_pos(pos);
		nlines++;
		if (!do_repaint)
		{
			home();
			add_line();
			put_line();
		}
	}

	eof_check();
	if (nlines == 0)
		eof_bell();
	else if (do_repaint)
		repaint();
	(void) currline(BOTTOM);
}

/*
 * Display n more lines, forward.
 * Start just after the line currently displayed at the bottom of the screen.
 */
	public void
forward(n, force, only_last)
	int n;
	int force;
	int only_last;
{
	POSITION pos;

	if (quit_at_eof && hit_eof && !(ch_getflags() & CH_HELPFILE))
	{
		/*
		 * If the -e flag is set and we're trying to go
		 * forward from end-of-file, go on to the next file.
		 */
		if (edit_next(1))
			quit(QUIT_OK);
		return;
	}

	pos = position(BOTTOM_PLUS_ONE);
	if (pos == NULL_POSITION && (!force || empty_lines(2, sc_height-1)))
	{
		if (ignore_eoi)
		{
			/*
			 * ignore_eoi is to support A_F_FOREVER.
			 * Back up until there is a line at the bottom
			 * of the screen.
			 */
			if (empty_screen())
				pos = ch_zero();
			else
			{
				do
				{
					back(1, position(TOP), 1, 0);
					pos = position(BOTTOM_PLUS_ONE);
				} while (pos == NULL_POSITION);
			}
		} else {
			eof_bell();
			hit_eof++;
			return;
		}
	}
	forw(n, pos, force, only_last, 0);
}

/*
 * Display n more lines, backward.
 * Start just before the line currently displayed at the top of the screen.
 */
	public void
backward(n, force, only_last)
	int n;
	int force;
	int only_last;
{
	POSITION pos;

	pos = position(TOP);
	if (pos == NULL_POSITION && (!force || position(BOTTOM) == 0))
	{
		eof_bell();
		return;   
	}
	back(n, pos, force, only_last);
}

/*
 * Get the backwards scroll limit.
 * Must call this function instead of just using the value of
 * back_scroll, because the default case depends on sc_height and
 * top_scroll, as well as back_scroll.
 */
	public int
get_back_scroll()
{
	if (no_back_scroll)
		return (0);
	if (back_scroll >= 0)
		return (back_scroll);
	if (top_scroll)
		return (sc_height - 2);
	return (10000); /* infinity */
}
