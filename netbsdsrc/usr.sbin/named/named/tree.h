/*	$NetBSD: tree.h,v 1.3 1997/10/04 15:12:19 mrg Exp $	*/

/* tree.h - declare structures used by tree library
 *
 * vix 22jan93 [revisited; uses RCS, ANSI, POSIX; has bug fixes]
 * vix 27jun86 [broken out of tree.c]
 *
 * from: Id: tree.h,v 8.1 1994/12/15 06:24:14 vixie Exp 
 */


#ifndef	_TREE_H_INCLUDED
#define	_TREE_H_INCLUDED


#ifndef __P
# if defined(__STDC__) || defined(__GNUC__)
#  define __P(x) x
# else
#  define __P(x) ()
# endif
#endif

/*
 * tree_t is our package-specific anonymous pointer.
 */
#if defined(__STDC__) || defined(__GNUC__)
typedef	void *tree_t;
#else
typedef	char *tree_t;
#endif


typedef	struct tree_s {
		tree_t		data;
		struct tree_s	*left, *right;
		short		bal;
	}
	tree;


void	tree_init	__P((tree **));
tree_t	tree_srch	__P((tree **, int (*)(), tree_t));
tree_t	tree_add	__P((tree **, int (*)(), tree_t, void (*)()));
int	tree_delete	__P((tree **, int (*)(), tree_t, void (*)()));
int	tree_trav	__P((tree **, int (*)()));
void	tree_mung	__P((tree **, void (*)()));


#endif	/* _TREE_H_INCLUDED */
