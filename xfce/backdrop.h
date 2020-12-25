#ifndef FD_backdrp_h_
#define FD_backdrp_h_
/* Header file generated with fdesign. */

/** Callback routines and free object handlers **/



/**** Forms and Objects ****/

typedef struct {
	FL_FORM *backdrp;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *preview;
	FL_OBJECT *load_btn;
	FL_OBJECT *apply_btn;
	FL_OBJECT *clear_btn;
	FL_OBJECT *exit_btn;
	FL_OBJECT *filename_inp;
} FD_backdrp;

extern FD_backdrp * create_form_backdrp(void);

#endif /* FD_backdrp_h_ */
