<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!-- saved from url=(0046)http://www.home.unix-ag.org/simon/gimp/quant.c -->
<HTML><HEAD>
<META content="text/html; charset=windows-1252" http-equiv=Content-Type>
<META content="MSHTML 5.00.2920.0" name=GENERATOR></HEAD>
<BODY><XMP>/*
 * This is a plug-in for the GIMP.
 *
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 * Copyright (C) 1996 Matthias Clasen
 * Copyright (C) 1998 Simon Budig (ported to 0.99.x)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Porting to 0.99.17 was done via "cut'n'paste" programming.
 * Nearly the complete quant-algorithm is untouched (except a nasty
 * bug, which crashed the plug-in sometimes...).
 * Code was taken from exchange.c, mblur.c, gauss_iir.c, and pixelize.c
 * Thanks to the authors.
 *         Simon Budig
 */

/* 
 * TODO:
 * - clean up code, some variables are e.g. used exactly once... ;
 * - the plug-in crashes, when called with 1 color and principal
 *   quantisation. Maybe it is similar to the other nasty bug.
 * - make the build_histogram() function ignore the fully transparent/
 *   unselected pixels. Some pictures give strange results:
 *     +-------------------+
 *     | red   white  blue |    Now select the white and the red
 *     | white white white |    field and run quant with 2 colors.
 *     | yello white green |    the blue, green and yellow fields
 *     +-------------------+    will be included in the histogram
 *   (the rectangular bounds of the selection are taken to compute
 *   it) and the red will turn to a kind of brown.
 */

/* 
 * This filter implements a variation of the quantization algorithm that is 
 * described in the article:  
 * Xiaolin Wu, Color Quantization by Dynamic Programming and Principal Analysis,
 * ACM Transactions on Graphics, Vol. 11, No. 4 (1992).
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <libgimp/gimp.h>

#define PLUG_IN_NAME    "plug_in_quant"
#define PLUG_IN_VERSION "Mar 1998, 0.96"

#define ENTRY_WIDTH 100

/* define this to get some messages to stdout */

#undef PROGRESS_MESSAGES


/* hash table entry */
typedef struct hash_table_entry_struct {
  unsigned char c[3];
  int num;
  struct hash_table_entry_struct *next;
} hash_table_entry;

/* color entry in work space with multiplicity and key 
 for sorting wrt principal axis */
typedef struct {
  float c[3];
  float key;
  int num;
} color;

typedef struct {
  int first;	/* first color */
  int num;	/* number of colors in the sample */ 
  int weight;  /* number of pixels in the sample */
  float u[3];	/* centroid of the sample in work space */
  unsigned char ci[3]; /* centroid in image space */
  float error;	/* quantisation error for the sample */
  float gain;  /* gain of splitting this sample in c1, c2 */
  unsigned char status; /* may be UNUSED, PRELEAF, LEAF, FINALLEAF */
  int c1, c2;   /* children */
} sample;

/* values for status of samples */
#define UNUSED           0
#define PRELEAF          1
#define LEAF	         2
#define FINALLEAF        3

/* algorithmic parameters, most of them are not user-settable in the current
   version for two reasons:
   - I don't know how to design clever dialogs for that purpose 
   - I don't know if they are useful at all. I just took some initial values
     (partially from Wu's article). Someone has to do some experiments with
     the parameters to determine the influential parameters and to find good 
     initial values and sensible ranges for them. I don't have the time.  
    */
typedef struct {
  /* basic */
  int K;             /* number of remaining colors */ 
  int rgb, luv;      /* workspace */
  /* principal quantization */
  int principal;     /* do it ? */
  int resolution;    /* resolution of sorting on principal axis */
  int additional;    /* split on if no bias ? */
  float min_bias;    /* minimal ratio of largest/smallest eigenvalue */
  float max_deviation;  /* maximal deviation between principal axes */ 
  /* k-means clustering */
  int kmeans;        /* do it ? this also affects the 2-clustering */
  int max_iterations;  /* maximal number of loops over the data points */
  /* CIE Luv */
  float xr, yr;     /* the chromaticity coordinates of the red phospor */
  float xg, yg;     /* the chromaticity coordinates of the green phospor */
  float xb, yb;     /* the chromaticity coordinates of the blue phospor */
  float xw, yw, Yw; /* CIE XYZ coordinates for white (R=G=B=1) */ 
} QuantValues;

/*********************************/
/* Prototypes                    */
/*********************************/

static void query(void);
static void run(char    *name,
                int      nparams,
                GParam  *param,
                int     *nreturn_vals,
                GParam **return_vals);

/* dialog callbacks */
static void    dialog_close_callback(GtkWidget *, gpointer);
static void    dialog_ok_callback(GtkWidget *, gpointer);
static void    dialog_cancel_callback(GtkWidget *, gpointer);
static void    dialog_entry_callback(GtkWidget *, gpointer);
static void    dialog_check_update(GtkWidget *, gint32);
static void    dialog_toggle_update(GtkWidget *, gint32);
static gint    quant_dialog(void);

/* color space conversion */
static void init_CIE ();
static void goto_workspace (unsigned char [3], float [3]);
static void goto_imagespace (float [3], unsigned char [3]);

/* matrix decomposition and inversion */
static void spectral (float [3][3], float [3][3], float [3]);
static void invert (float [3][3], float [3][3]); 

/* building a histogram of the source image */
static void build_histogram (GDrawable *);

/* mapping the source image to the destination image */
static void map_hashed (GDrawable *, sample *, int);

/* auxiliary functions */
static float calc_axis (int, float [3]);
static void calc_centroid (int);
static void calc_error (int);
static void sort_along_axis (float [3], int, int);

/* binary splitting */
static void binary_split (int);

/* splitting along the principal axis */
static int principal_split ();

/* main function */
static void quant (GDrawable *);


/***** Variables *****/

GPlugInInfo PLUG_IN_INFO = {
        NULL,   /* init_proc */
        NULL,   /* quit_proc */
        query,  /* query_proc */
        run     /* run_proc */
}; /* PLUG_IN_INFO */

static QuantValues vals;
static quant_run = FALSE;
color *colors;
int max_color;
sample *samples;
int max_sample;

/************* Functions ***************/

/****/
MAIN()
/****/

static void query(void) {
  static GParamDef args[] = {
    { PARAM_INT32,    "run_mode",  "Interactive, non-interactive" },
    { PARAM_IMAGE,    "image",     "Input image" },
    { PARAM_DRAWABLE, "drawable",  "Input drawable" },
    { PARAM_INT32,    "numcols",   "Number of colors" },
    { PARAM_INT32,    "type",      "Color Space (0 - RGB, 1 - CIE Luv)" },
    { PARAM_INT32,    "principal", "Principal quantisation" },
  }; /* args */

  static GParamDef *return_vals  = NULL;
  static int        nargs        = sizeof(args) / sizeof(args[0]);
  static int        nreturn_vals = 0;

  gimp_install_procedure(PLUG_IN_NAME,
                         "Quantisation of Colors",
                         "This plug-in reduces colors to a given number.",
                         "Matthias Clasen and Simon Budig",
                         "Matthias Clasen and Simon Budig",
   
                         PLUG_IN_VERSION,
                         "<Image>/Filters/Colors/Quantize",
                         "RGB*",
                         PROC_PLUG_IN,
                         nargs,
                         nreturn_vals,
                         args,
                         return_vals);
} /* query */


/********************************************************************/
/* User Interface                                                   */
/********************************************************************/

static gint quant_dialog(void) {
   GtkWidget      *dialog;
   GtkWidget      *csframe, *vbox, *ivbox, *hbox;
   GtkWidget      *princ_chkbox, *nc_entry;
   GtkWidget      *button, *label;

   gint           argc;
   gchar          **argv;
   gchar          buffer[256];

   argc    = 1;
   argv    = g_new(gchar *, 1);
   argv[0] = g_strdup("quantize");

   gtk_init(&argc, &argv);
   gtk_rc_parse(gimp_gtkrc());
   gdk_set_use_xshm(gimp_use_xshm());

   dialog = gtk_dialog_new();
   gtk_window_set_title(GTK_WINDOW(dialog), "Quantize colors");
   gtk_container_border_width(GTK_CONTAINER(dialog), 0);
   gtk_signal_connect(GTK_OBJECT(dialog), "destroy",
                     (GtkSignalFunc) dialog_close_callback,
                     NULL);

   button = gtk_button_new_with_label("OK");
   GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
   gtk_signal_connect(GTK_OBJECT(button), "clicked",
                      (GtkSignalFunc) dialog_ok_callback,
                      dialog);
   gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), button, TRUE, TRUE, 0);
   gtk_widget_grab_default(button);
   gtk_widget_show(button);
 
   button = gtk_button_new_with_label("Cancel");
   GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
   gtk_signal_connect(GTK_OBJECT(button), "clicked",
                      (GtkSignalFunc) dialog_cancel_callback,
                      dialog);
   gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), button, TRUE, TRUE, 0);
   gtk_widget_show(button);
 
   vbox = gtk_vbox_new(FALSE, 5);
   gtk_container_border_width (GTK_CONTAINER (vbox), 5);
   gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
                       vbox, FALSE,FALSE,0);
 
   hbox = gtk_hbox_new (FALSE, 5);
   gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
   gtk_widget_show (hbox); 

   label = gtk_label_new ("Number of colors: ");
   gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, FALSE, 0);
   gtk_widget_show (label);
 
   nc_entry = gtk_entry_new ();
   gtk_box_pack_start (GTK_BOX (hbox), nc_entry, TRUE, TRUE, 0);
   gtk_widget_set_usize (nc_entry, ENTRY_WIDTH, 0);
   sprintf (buffer, "%d", vals.K);
   gtk_entry_set_text (GTK_ENTRY (nc_entry), buffer);
   gtk_signal_connect (GTK_OBJECT (nc_entry), "changed",
                       (GtkSignalFunc) dialog_entry_callback,
                       NULL);
   gtk_widget_show (nc_entry);
    

   princ_chkbox = gtk_check_button_new_with_label ("Principal quantisation");
   gtk_box_pack_start (GTK_BOX (vbox), princ_chkbox, TRUE, TRUE, 0);
   gtk_signal_connect (GTK_OBJECT (princ_chkbox), "toggled",
                       (GtkSignalFunc) dialog_check_update,
                       NULL);
   gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (princ_chkbox), vals.principal);
   gtk_widget_show (princ_chkbox);


   /*****/
 
   csframe= gtk_frame_new("Color Space");
   gtk_frame_set_shadow_type(GTK_FRAME(csframe), GTK_SHADOW_ETCHED_IN);
   gtk_box_pack_start(GTK_BOX(vbox),
                      csframe, FALSE, FALSE, 0);
 
   ivbox= gtk_vbox_new(FALSE, 5);
   gtk_container_border_width (GTK_CONTAINER (ivbox), 5);
   gtk_container_add(GTK_CONTAINER(csframe), ivbox);
  
   {
     int   i;
     char * name[2]= {"RGB", "CIE Luv"};
 
     button= NULL;
     for (i=0; i < 2; i++)
        {
           button= gtk_radio_button_new_with_label(
              (button==NULL)? NULL :
                 gtk_radio_button_group(GTK_RADIO_BUTTON(button)), 
              name[i]);
           gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), 
                                       (i==0?(vals.rgb==1?TRUE:FALSE):
                                            (vals.luv==1?TRUE:FALSE)));
   
           gtk_signal_connect (GTK_OBJECT (button), "toggled",
                               (GtkSignalFunc) dialog_toggle_update,
                               (gpointer) i);
   
           gtk_box_pack_start(GTK_BOX(ivbox), button, FALSE, FALSE,0);
           gtk_widget_show(button);
        }
   }
 
   gtk_widget_show(ivbox);
   gtk_widget_show(csframe);
 
   /*****/
  
   gtk_widget_show(vbox);
 
   gtk_widget_show(dialog);
 
   gtk_main();  
   gdk_flush();
 
   return quant_run;
}
 
/********************************************************************/
/* dialog callbacks                                                 */
/********************************************************************/

static void
dialog_close_callback(GtkWidget *widget, gpointer data)
{
  gtk_main_quit();
}

static void
dialog_ok_callback(GtkWidget *widget, gpointer data)
{
  quant_run= TRUE;
  gtk_widget_destroy(GTK_WIDGET(data));
}

static void
dialog_cancel_callback(GtkWidget *widget, gpointer data)
{
  gtk_widget_destroy(GTK_WIDGET(data));
}

static void
dialog_entry_callback (GtkWidget *widget,
                      gpointer   data)
{
  vals.K = atoi (gtk_entry_get_text (GTK_ENTRY (widget)));
  if (vals.K < 1) {
    vals.K = 1;
  }
}

static void
dialog_toggle_update(GtkWidget *widget, gint32 value)
{
   if (value==0) 
     vals.rgb = (GTK_TOGGLE_BUTTON (widget)->active)?1:0;
   else
     vals.luv = (GTK_TOGGLE_BUTTON (widget)->active)?1:0;
}

static void
dialog_check_update(GtkWidget *widget, gint32 value)
{
   if (GTK_TOGGLE_BUTTON (widget)->active)
     vals.principal=1;
   else
     vals.principal=0;
}

/********************************************************************/
/* main function                                                    */
/********************************************************************/

#define DEG_to_RAD(x) ((x)*M_PI/180.0)
#define RAD_to_DEG(x) (180.0*(x)/M_PI)

static void
run(gchar       *prog_name,
    gint        nparams,
    GParam      *param,
    gint        *nreturn_vals,
    GParam      **return_vals)
{
  static GParam values[1];
  GDrawable *drawable;
  GRunModeType run_mode;
  GStatusType status = STATUS_SUCCESS;

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals = values;

  values[0].type = PARAM_STATUS;
  values[0].data.d_status = status;

   /* Presets */
   /* basic */
   vals.K=256;
   vals.rgb=1;
   vals.luv=0;
   /* principal quantization */ 
   vals.principal=1;
   vals.max_deviation= cos (DEG_to_RAD(15)); /* tolerate 15 degree */
   vals.min_bias=1.1; /* minimal ratio beween largest eigenvalues */
   vals.resolution=1024;
   vals.additional=0;
   /* k-means clustering */
   vals.kmeans=1;
   vals.max_iterations=100;
   /* CIE Luv */
   vals.xr=0.67; vals.yr=0.33;  /* NTSC Phosphors: red   */
   vals.xg=0.21; vals.yg=0.71;  /*  green */ 
   vals.xb=0.14; vals.yb=0.08;  /*  blue  */
   vals.xw=0.31; vals.yw=0.316; vals.Yw=100.0;  /* illuminant C */

   /*  Possibly retrieve data  */
   gimp_get_data (PLUG_IN_NAME, &vals);

   switch (run_mode) {
   case RUN_INTERACTIVE:

      /*  First acquire information with a dialog  */
      if (! quant_dialog ())
        return;
      break;

   case RUN_NONINTERACTIVE:
      /*  Make sure all the arguments are there!  */
      if (nparams != 6)
        status = STATUS_CALLING_ERROR;
      if (status == STATUS_SUCCESS)
        {
          vals.K = param[3].data.d_int32;
          vals.rgb = (param[4].data.d_int32) ? 0 : 1;
          vals.luv = (param[4].data.d_int32) ? 1 : 0;
          vals.principal = (param[5].data.d_int32) ? TRUE : FALSE;
        }
      if (status == STATUS_SUCCESS && (vals.K < 1))
        if ((vals.rgb==1 && vals.luv==0) || (vals.rgb==0 && vals.luv==1))
          status = STATUS_CALLING_ERROR;
      break;

    case RUN_WITH_LAST_VALS:
      break;

    default:
      break;
    }

  /*  Get the specified drawable  */
  drawable = gimp_drawable_get (param[2].data.d_drawable);

  /*  Make sure that the drawable is RGB color  */
  if (gimp_drawable_color (drawable->id))
    {
      /*  run the quantization  */
      quant (drawable);

      if (run_mode != RUN_NONINTERACTIVE)
        gimp_displays_flush ();

      /*  Store data  */
      if (run_mode == RUN_INTERACTIVE)
        gimp_set_data (PLUG_IN_NAME, &vals, sizeof (QuantValues));
    }
  else
    {
      /* quant cannot operate on indexed color or gray images */
      status = STATUS_EXECUTION_ERROR;
    }

  values[0].data.d_status = status;

  gimp_drawable_detach (drawable);
}



/********************************************************************/
/* color space conversions                                          */
/********************************************************************/

/*
 * This code is based on information found in          
 * Foley, van Dam, Feiner and Hughes, Computer Graphics, 2. Ed, Chapter 13
 * The treatment of small Y in the color space transformations     
 * is not from somewhere else.
 */

static float RGB2XYZ[3][3];
static float XYZ2RGB[3][3];
static float YWhite, uWhite, vWhite;

static void init_CIE ()
{
  float Xw, Zw;
  float U[3][3], Uinv[3][3], C[3]; 
  int i, j;

    Xw = vals.xw / vals.yw * vals.Yw;
    Zw = (1 - vals.xw - vals.yw) / vals.yw * vals.Yw;
    U[0][0] = vals.xr; 
    U[0][1] = vals.xg; 
    U[0][2] = vals.xb;
    U[1][0] = vals.yr; 
    U[1][1] = vals.yg; 
    U[1][2] = vals.yb;
    U[2][0] = 1-vals.xr-vals.yr; 
    U[2][1] = 1-vals.xg-vals.yg; 
    U[2][2] = 1-vals.xb-vals.yb;
    
    /* obtain C by solving (Xw Yw Zw) = UC */
    invert(U,Uinv);
    C[0] = Uinv[0][0]*Xw + Uinv[0][1]*vals.Yw + Uinv[0][2]*Zw; 
    C[1] = Uinv[1][0]*Xw + Uinv[1][1]*vals.Yw + Uinv[1][2]*Zw; 
    C[2] = Uinv[2][0]*Xw + Uinv[2][1]*vals.Yw + Uinv[2][2]*Zw; 
    
    /* calculate the matrix for going from RGB to XYZ */
    RGB2XYZ[0][0]=U[0][0]*C[0];RGB2XYZ[0][1]=U[0][1]*C[1];RGB2XYZ[0][2]=U[0][2]*C[2];
    RGB2XYZ[1][0]=U[1][0]*C[0];RGB2XYZ[1][1]=U[1][1]*C[1];RGB2XYZ[1][2]=U[1][2]*C[2];
    RGB2XYZ[2][0]=U[2][0]*C[0];RGB2XYZ[2][1]=U[2][1]*C[1];RGB2XYZ[2][2]=U[2][2]*C[2];
    
    invert(RGB2XYZ,XYZ2RGB);
    
    for (i=0;i<3;i++) {
      for (j=0;j<3;j++) {
	RGB2XYZ[i][j]/=255.0;
	XYZ2RGB[i][j]*=255.0;
      }
    }
    YWhite = vals.Yw;
    uWhite = 4.0 * Xw / (Xw + 15.0 * vals.Yw + 3.0 * Zw);
    vWhite = 9.0 * vals.Yw / (Xw + 15.0 * vals.Yw + 3.0 * Zw);
}

static void goto_workspace (rgb, luv)
     unsigned char rgb [3];
     float luv [3];
{
  float X, Y, Z, d;

  if (vals.rgb) {
   /* go to normalized RGB */
   luv[0] = rgb[0] / 255.0; 
   luv[1] = rgb[1] / 255.0; 
   luv[2] = rgb[2] / 255.0;
  } else {
    /* go to CIE XYZ */
    X = RGB2XYZ[0][0]*rgb[0] + RGB2XYZ[0][1]*rgb[1] + RGB2XYZ[0][2]*rgb[2];
    Y = RGB2XYZ[1][0]*rgb[0] + RGB2XYZ[1][1]*rgb[1] + RGB2XYZ[1][2]*rgb[2];
    Z = RGB2XYZ[2][0]*rgb[0] + RGB2XYZ[2][1]*rgb[1] + RGB2XYZ[2][2]*rgb[2];
    /* go to CIE Luv */
    d = X + 15.0 * Y + 3.0 * Z;
    if (d>0) {
      luv[1] = 4.0 * X / d;
      luv[2] = 9.0 * Y / d;
    } else {
      luv[1] = 0;	
      luv[2] = 0;	
    }				
    if (Y / YWhite > 0.008856) { 	
      luv[0] = 116.0 * pow(Y / YWhite, 0.33333333) - 16.0;
    } else {				
      luv[0] = 903.0 * Y / YWhite;
    }			
    luv[1] = 13.0*(luv[0])*(luv[1] - uWhite);
    luv[2] = 13.0*(luv[0])*(luv[2] - vWhite);
  }
}

static void goto_imagespace (luv, rgb)
     float luv [3];
     unsigned char rgb [3];
{
  float X, Y, Z, d, uu, vv;
  
  if (vals.rgb) {
    /* go to unnormalized RGB */    
    rgb[0] = luv[0]*255.0; 
    rgb[1] = luv[1]*255.0; 
    rgb[2] = luv[2]*255.0;
  } else {
    /* go to CIE XYZ */
    if (luv[0] == 0) {
      X = Y = Z = 0;
    } else {
      if (luv[0] <= 7.996968) {
	Y = luv[0] / 903.0 * YWhite;
      } else {
	Y = pow((luv[0] + 16.0) / 116.0, 3.0)  * YWhite;
      }
      uu = luv[1] / (13.0 * luv[0]) + uWhite;
      vv = luv[2] / (13.0 * luv[0]) + vWhite;
      d = 9.0 * Y / vv;
      X = uu * d / 4.0;
      Z = (d - 15.0 * Y - X) / 3.0;
    }
    /* go to unnormalized RGB */
    rgb[0] = XYZ2RGB[0][0]*X + XYZ2RGB[0][1]*Y + XYZ2RGB[0][2]*Z; 
    rgb[1] = XYZ2RGB[1][0]*X + XYZ2RGB[1][1]*Y + XYZ2RGB[1][2]*Z; 
    rgb[2] = XYZ2RGB[2][0]*X + XYZ2RGB[2][1]*Y + XYZ2RGB[2][2]*Z; 
  }
}

/********************************************************************/
/* matrix decomposition and inversion                               */
/********************************************************************/

/* 
 * The spectral decomposition code is from
 * Ken Shoemake, Polar Matrix Decomposition, in Graphics Gems IV.
 * The inversion code is based on the classical adjoint for 3x3-matrices;
 * look in any book on linear algebra for that.
 */

static void spectral (S, U, K)
     float S[3][3], U[3][3], K[3];
{
  float OffD[3], sm, g, h, fabsh, fabsOffDi, t, theta;
  float c, s, tau, ta, OffDq, a, b;
  int sweep, i, j, p, q;

  for (i = 0; i < 3; i++) {
    U[i][i] = 1;
    K[i] = S[i][i];
    OffD[i] = S[((i + 1) % 3)][(i + 2) % 3];
    for (j = i + 1; j < 3; j++) {
      U[i][j] = U[j][i] = 0;
    }
  }
  for (sweep = 25; sweep > 0; sweep--) {
    sm = fabs(OffD[0]) + fabs(OffD[1]) + fabs(OffD[2]);
    if (sm == 0) break;
    for (i = 2; i >= 0; i--) {
      p = (i + 1) % 3;
      q = (p + 1) % 3;
      
      fabsOffDi = fabs(OffD[i]);
      g = 100.0 * fabsOffDi;
      if (fabsOffDi > 0) {
	h = K[q] - K[p];
	fabsh = fabs(h);
	if (fabsh + g == fabsh) {
	  t = OffD[i] / h;
	} else {
	  theta = 0.5 * h / OffD[i];
	  t = 1.0 / (fabs(theta) + sqrt(theta * theta + 1.0));
	  if (theta < 0) t = -t;
	}
	c = 1.0 / sqrt(t * t + 1);
	s = t * c;
	tau = s / (c + 1);
	ta = t * OffD[i];
	OffD[i] = 0.0;
	K[p] -= ta;
	K[q] += ta;
	OffDq = OffD[q];
	OffD[q] -= s * (OffD[p] + tau * OffD[q]);
	OffD[p] += s * (OffDq - tau * OffD[p]);
	for (j = 2; j >= 0; j--) {
	  a = U[j][p];
	  b = U[j][q];
	  U[j][p] -= s * (b + tau * a);
	  U[j][q] += s * (a - tau * b);
	}
      }
    }
  }
}

static void invert (A,B)
     float A[3][3], B[3][3]; 
{
  float det;
  B[0][0] =   (A[1][1]*A[2][2] - A[1][2]*A[2][1]);
  B[1][0] = - (A[1][0]*A[2][2] - A[1][2]*A[2][0]);
  B[2][0] =   (A[1][0]*A[2][1] - A[1][1]*A[2][0]);
  B[0][1] = - (A[0][1]*A[2][2] - A[0][2]*A[2][1]);
  B[1][1] =   (A[0][0]*A[2][2] - A[0][2]*A[2][0]);
  B[2][1] = - (A[0][0]*A[2][1] - A[0][1]*A[2][0]);
  B[0][2] =   (A[0][1]*A[1][2] - A[0][2]*A[1][1]);
  B[1][2] = - (A[0][0]*A[1][2] - A[0][2]*A[1][0]);
  B[2][2] =   (A[0][0]*A[1][1] - A[0][1]*A[1][0]);
  det = A[0][0]*B[0][0] + A[0][1]*B[1][0] + A[0][2]*B[2][0];
  if (det < 0.001) fprintf(stderr,"Near singular matrix.\n");
  B[0][0] /= det; B[0][1] /= det; B[0][2] /= det;
  B[1][0] /= det; B[1][1] /= det; B[1][2] /= det;
  B[2][0] /= det; B[2][1] /= det; B[2][2] /= det;
}


/********************************************************************/
/* building histogram of source image                               */
/********************************************************************/

/* 
 * The next two functions implement histogram building and nearest neighbour 
 * mapping using a hash table with separate chaining. This code is strongly 
 * based on the corresponding parts of libppm, see the copyright notice below.
 */ 

/* libppm3.c - ppm utility library part 3
**
** Colormap routines.
**
** Copyright (C) 1989, 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#define HASH_SIZE 20023
#define HASH_FUNC(c) ((((long) c[0] * 33023 + \
			(long) c[1] * 30013 + \
			(long) c[2] * 27011 ) & 0x7fffffff ) % HASH_SIZE )

static void build_histogram (GDrawable *drawable)
{
  hash_table_entry  **hash_table, *entry, *next; 
  guchar *sp, *src;
  int hash;
  GPixelRgn src_rgn;
  guint x, y;
  gint progress, max_progress;
  gint x1, x2, y1, y2;
  gpointer pr;
  color *c;

  gimp_drawable_mask_bounds (drawable->id, &x1, &y1, &x2, &y2);
  gimp_pixel_rgn_init (&src_rgn, drawable, x1, y1, x2-x1, y2-y1, FALSE, FALSE);

  progress = 0;
  max_progress = (x2-x1)*(y2-y1);

  hash_table = (hash_table_entry **) malloc (HASH_SIZE*sizeof(hash_table_entry *));
  for (hash = 0; hash < HASH_SIZE; hash++) { hash_table[hash] = NULL; }
  max_color = 0;

  gimp_progress_init ("(1/3) Building histogram...");

  for (pr = gimp_pixel_rgns_register (1, &src_rgn); pr != NULL; pr = gimp_pixel_rgns_process (pr)) {
    src=src_rgn.data;
    for (y = 0; y < src_rgn.h; y++) {
      sp=src;
      for (x = 0; x < src_rgn.w; x++, sp += src_rgn.bpp) {
        hash = HASH_FUNC(sp);      
        for (entry = hash_table[hash]; ; entry = entry->next) {
          if (entry == NULL) {
	    entry = (hash_table_entry *) malloc (sizeof(hash_table_entry));
	    entry->c[0] = sp[0];
	    entry->c[1] = sp[1];
	    entry->c[2] = sp[2];
	    entry->num = 1;
	    entry->next = hash_table[hash];
	    hash_table[hash] = entry;
	    max_color++;
	    break; /* we've inserted it */
	  } else if ((entry->c[0] == sp[0]) &&
		     (entry->c[1] == sp[1]) &&
		     (entry->c[2] == sp[2])) { 
	    entry->num++;
	    break; /* we've found it */
	  }
        }
      }
      src += src_rgn.rowstride;
    }
    progress += src_rgn.w * src_rgn.h;
    gimp_progress_update ((double) progress / (double) max_progress);
  }

#ifdef PROGRESS_MESSAGES
  printf("%d colors found\n", max_color);
#endif

  colors = (color *) malloc (max_color * sizeof (color));

  for (hash = 0, c = colors; hash < HASH_SIZE; hash++) {
    for (entry = hash_table[hash]; entry != NULL; entry = entry->next, c++) {
      goto_workspace (entry->c, c->c);
      c->num = entry->num;
    }
  }
  
  for (hash = 0; hash < HASH_SIZE; hash++) {
    for (entry = hash_table[hash]; entry != NULL; entry = next) {
      next = entry->next;
      free (entry);
    }
  }
  free (hash_table);
}

static void map_hashed (GDrawable *drawable, sample *lut, int len)
{
  hash_table_entry **hash_table, *entry, *hash_store, *next_free;
  guchar *sp, *src;
  guchar *dp, *dest;
  int hash;
  float dist, newdist, cs[3];
  sample *s, *last_s, t, v;
  float sdist [len][len], mdist[len];
  int i, j, idx;
  GPixelRgn src_rgn, dest_rgn;
  gpointer pr;
  gint x1,x2,y1,y2;
  guint x,y, rest;
  gint progress,max_progress;

  gimp_drawable_mask_bounds (drawable->id, &x1, &y1, &x2, &y2);

  gimp_pixel_rgn_init (&src_rgn, drawable, x1, y1, x2-x1, y2-y1, FALSE, FALSE);
  gimp_pixel_rgn_init (&dest_rgn, drawable, x1, y1, x2-x1, y2-y1, TRUE, TRUE);

  progress=0;
  max_progress=(x2-x1)*(y2-y1);

  /* 
     sort samples according to their number of colors;
     here we use insertion sort since the number of samples
     will usually be small */

  if (lut[0].num < lut[1].num) { 
    t = lut[1]; lut[1] = lut[0]; lut[0] = t; 
  }
  t = lut[0]; lut[0].num = 20000000; /* sentinel key */
  
  for (i = 1; i < len; i++) {
    v = lut[i];
    if (lut[i-1].num < v.num) {
      j = i;
      do {
	lut[j] = lut[j-1]; j--; /* shift up */
      } while (lut[j-1].num < v.num);
      lut[j] = v; /* insert v */
    }
  }
  /* re-insert t; this works without sentinel key, since t is not the largest */
  i = 0;
  while (lut[i+1].num > t.num) {
    lut[i] = lut[i+1]; i++;
  }
  lut[i] = t;

  /* calculate the squared halfdistances between all centroids (sdist)
     and the radii of the spheres belonging to each centroid (mdist)
     these can be used to save some comparisons in nearest-neighbour search */
  for (i = 0; i < len; i++) { mdist[i] = 1E20; }
  for (i = 0; i < len; i++) {
    sdist[i][i] = 0;
    for (j = i + 1; j < len; j++) {
      sdist[j][i] = sdist[i][j] =
	( (lut[i].u[0]-lut[j].u[0])*(lut[i].u[0]-lut[j].u[0])
	 +(lut[i].u[1]-lut[j].u[1])*(lut[i].u[1]-lut[j].u[1])
	 +(lut[i].u[2]-lut[j].u[2])*(lut[i].u[2]-lut[j].u[2]))/4;
      if (sdist[j][i]<mdist[i]) { mdist[i] = sdist[j][i]; }
      if (sdist[j][i]<mdist[j]) { mdist[j] = sdist[j][i]; }
    }
  }
  
  hash_table = (hash_table_entry **) malloc (HASH_SIZE * sizeof(hash_table_entry *));
  for (hash = 0; hash < HASH_SIZE; hash++) { hash_table[hash] = NULL; }

  /* avoid many malloc's, since we know the number of entries in advance */
  hash_store = (hash_table_entry *) malloc (max_color * sizeof(hash_table_entry));
  next_free = hash_store;
  
  last_s = lut + len;
  gimp_progress_init("(3/3) Applying color map...");

  for (pr = gimp_pixel_rgns_register (2, &src_rgn, &dest_rgn); pr != NULL; pr = gimp_pixel_rgns_process (pr)) {
    src = src_rgn.data;
    dest = dest_rgn.data;
    for (y = 0; y < src_rgn.h; y++) {
      sp = src;
      dp = dest;
      for (x = 0; x < src_rgn.w; x++) {
        hash = HASH_FUNC(sp);
        idx = -1;
        for (entry = hash_table[hash]; entry != NULL; entry = entry->next) {
	  if (   (entry->c[0] == sp[0]) 
	      && (entry->c[1] == sp[1]) 
	      && (entry->c[2] == sp[2])  ) { 
	    idx = entry->num;
	    break; 
	  }
        }
        if (idx < 0) {
	  dist = 1E20;
	  idx = 0;
	  goto_workspace (sp, cs);
	  for (s = lut, i = 0; s != last_s; s++, i++) { 
	    if (dist >= sdist[idx][i]) { /* triangle inequation */
	      newdist = ((cs[0] - s->u[0])*(cs[0] - s->u[0]) + 
		         (cs[1] - s->u[1])*(cs[1] - s->u[1]) + 
		         (cs[2] - s->u[2])*(cs[2] - s->u[2]));
	      if (newdist < dist) {
	        idx = i;
	        dist = newdist;
	        if (dist < mdist[i]) { break; } /* cs in sphere of s->u */
	      }
	    } 
	  }
	  next_free->next = hash_table[hash];
	  hash_table[hash] = next_free;
	  next_free->c[0] = sp[0];
	  next_free->c[1] = sp[1];
	  next_free->c[2] = sp[2];
	  next_free->num = idx;
	  next_free++;
        }

        dp[0] = lut[idx].ci[0];
        dp[1] = lut[idx].ci[1];
        dp[2] = lut[idx].ci[2];
        for (rest = 3; rest < src_rgn.bpp; rest++) dp[rest] = sp[rest];
        sp += src_rgn.bpp;
        dp += dest_rgn.bpp;
      }
      src += src_rgn.rowstride;
      dest += dest_rgn.rowstride;
    }
    progress += src_rgn.w*src_rgn.h;
    gimp_progress_update((double) progress/ (double) max_progress);
  }
  free (hash_store);
  free (hash_table);
}

/********************************************************************/
/* auxiliary functions                                              */
/********************************************************************/

static float calc_axis (idx, axis)
     int idx;
     float axis[3];
{
  int princ, princ2;
  float Cov[3][3], U[3][3], lambda[3];
  color *first, *last, *c;

  /* calculate the covariance matrix of the sample */
  Cov[0][0] = Cov[0][1] = Cov[0][2] = 0;
  Cov[1][1] = Cov[1][2] = Cov[2][2] = 0;      

  first = colors + samples[idx].first;
  last = first + samples[idx].num;
  for (c = first; c != last; c++) {
    Cov[0][0] += c->num * (c->c[0]*c->c[0]);
    Cov[0][1] += c->num * (c->c[0]*c->c[1]);
    Cov[0][2] += c->num * (c->c[0]*c->c[2]);
    Cov[1][1] += c->num * (c->c[1]*c->c[1]);
    Cov[1][2] += c->num * (c->c[1]*c->c[2]);
    Cov[2][2] += c->num * (c->c[2]*c->c[2]);
  }
  Cov[0][0] = (Cov[0][0]/samples[idx].weight)-(samples[idx].u[0]*samples[idx].u[0]);
  Cov[0][1] = (Cov[0][1]/samples[idx].weight)-(samples[idx].u[0]*samples[idx].u[1]);
  Cov[0][2] = (Cov[0][2]/samples[idx].weight)-(samples[idx].u[0]*samples[idx].u[2]);
  Cov[1][1] = (Cov[1][1]/samples[idx].weight)-(samples[idx].u[1]*samples[idx].u[1]);
  Cov[1][2] = (Cov[1][2]/samples[idx].weight)-(samples[idx].u[1]*samples[idx].u[2]);
  Cov[2][2] = (Cov[2][2]/samples[idx].weight)-(samples[idx].u[2]*samples[idx].u[2]);
  Cov[1][0] = Cov[0][1]; Cov[2][0] = Cov[0][2]; Cov[2][1] = Cov[1][2]; 

  /* do a spectral decomposition of Cov */
  spectral (Cov, U, lambda);

  /* find the largest eigenvalue, and the corr. eigenvector */
  if (lambda[0] < 0) { lambda[0] = - lambda[0]; }
  if (lambda[1] < 0) { lambda[1] = - lambda[1]; }
  if (lambda[2] < 0) { lambda[2] = - lambda[2]; }
 
  princ = 0;
  if (lambda[1] > lambda[0]) { princ = 1; }
  if (lambda[2] > lambda[princ]) { princ = 2; }
  princ2 = (princ+1) % 3; 
  if (lambda[(princ+2) % 3] > lambda[princ2]) { princ2 = (princ+2) % 3; }

  axis[0] = U[0][princ];
  axis[1] = U[1][princ];
  axis[2] = U[2][princ];   
  
  if (lambda[princ2]!=0) {
    return lambda[princ]/lambda[princ2];
  } else {
     return 1E20; /* infinity */
  }  
}

static void calc_centroid (idx)
     int idx;
{
  sample *s;
  color *first, *last, *c;
 
  s = samples + idx;
  s->u[0] = s->u[1] = s->u[2] = 0;
  s->weight = 0;
  first = colors + s->first; 
  last = first + s->num;
  for (c = first; c != last; c++) {
    s->u[0] += c->num * c->c[0];
    s->u[1] += c->num * c->c[1];
    s->u[2] += c->num * c->c[2];
    s->weight += c->num;
  }
  s->u[0] /= s->weight;	
  s->u[1] /= s->weight;
  s->u[2] /= s->weight;
}

static void calc_error (idx)
     int idx;
{
  sample *s;
  color *first, *last, *c;

  s = samples + idx;
  s->error = 0;
  first = colors + s->first;
  last = first + s->num;
  for (c = first; c != last; c++) {
    s->error += c->num * (  (c->c[0] - s->u[0]) * (c->c[0] - s->u[0])
			  + (c->c[1] - s->u[1]) * (c->c[1] - s->u[1])
			  + (c->c[2] - s->u[2]) * (c->c[2] - s->u[2]) );
  }
  s->error *= s->weight;
}

/* 
 * Elaborate quicksort a la Segewick, Algorithms, Chapter 9.
 */

static void sort_along_axis (axis, start, num)
     float axis[3];
     int start, num;
{
  int i, j, l, r;
  color *first, *last, *c, v, t;
  int top, stack[60]; /* much more than enough */

  if (num>1) {
    /* projecting onto axis */
    first = colors + start;
    last = first + num;
    for (c = first; c != last; c++) {
      c->key = c->c[0]*axis[0] + c->c[1]*axis[1] + c->c[2]*axis[2];
    }

    /* quicksort */
    l = start; r = start + num - 1;
    top = 0;
    stack[top] = l; top++;
    stack[top] = r; top++;
    while (top > 0) { /* stack empty <==> top==0 */
      if (r-l>10) { /* M==10 is just a guess, Sedgewick recommends 5 < M < 25 */
	/* sort colors[l], colors[(r+l)/2] and colors[r] */
	i = (r+l)/2;
	if (colors[l].key>colors[i].key) { 
	  t = colors[i]; colors[i] = colors[l]; colors[l] = t; 
	}
	if (colors[l].key>colors[r].key) { 
	  t = colors[r]; colors[r] = colors[l]; colors[l] = t; 
	}
	if (colors[i].key>colors[r].key) { 
	  t = colors[r]; colors[r] = colors[i]; colors[i] = t; 
	}
	
	/* exchange colors[(r+l)/2] and colors[r-1] and make v == colors[r-1] */
	v = colors[i]; colors[i] = colors[r-1]; colors[r-1] = v;
	
	/* i = partition(l+1,r-1) */
	i = l; j = r-1;
	do {
	  do { i++; } while (v.key > colors[i].key);
	  do { j--; } while (v.key < colors[j].key);
	  t = colors[i]; colors[i] = colors[j]; colors[j] = t;
	} while (i<j);
	colors[j] = colors[i]; colors[i] = colors[r-1]; colors[r-1] = t;
	
	/* recursion: put the larger part on the stack */
	if (i-l>r-i) {
	  stack[top] = l; top++;
	  stack[top] = i-1; top++;
	  l = i+1;
	} else {
	  stack[top] = i+1; top++;
	  stack[top] = r; top++;
	  r = i-1;
	}
      } else {
	top--; r = stack[top];
	top--; l = stack[top];
      }
    } /* while */
    
    /* insertion sort */
    /* make sure colors[start] is not the largest element */
    if (colors[start].key>colors[start+1].key) { 
      t = colors[start+1]; colors[start+1] = colors[start]; colors[start] = t; 
    }
    t = colors[start]; colors[start].key = -1E20; /* sentinel key */

    for (i = start + 1; i < start + num; i++) {
      v = colors[i];
      if (colors[i-1].key>v.key) {
	j = i;
	do {
	  colors[j] = colors[j-1]; j--; /* shift up */
	} while (colors[j-1].key>v.key);
	colors[j] = v; /* insert v */
      }
    }
    /* re-insert t; this works without sentinel key, since t is not the largest */
    i = start;
    while (colors[i+1].key<t.key) {
      colors[i] = colors[i+1]; i++;
    }
    colors[i] = t;
  } /* if more than one element */
}

/********************************************************************/
/* binary splitting                                                 */
/********************************************************************/

static void binary_split (idx)
     int idx;
{
  int xi, convergence, it;
  int acc_num, opt_num;
  float max[3], acc[3], opt[3], normal[3], ofs, cofs, value, val_i, axis[3];
  color *c, ctmp, *first, *last;
  sample *s, *c1, *c2;

  s = samples + idx;

  if (s->num == 1) {
    s->status = FINALLEAF; /* a leaf that can not be split any more */
    return; 
  }

  /* find free samples for the children of s */
  for (s->c1 = 0; s->c1 < max_sample; s->c1++) {
    if (samples[s->c1].status == UNUSED) { break; }
  }
  c1 = samples + s->c1; 
  c1->status = PRELEAF;

  for (s->c2 = 0; s->c2 < max_sample; s->c2++) {
    if (samples[s->c2].status == UNUSED) { break; }
  }
  c2 = samples + s->c2; 
  c2->status = PRELEAF;

  /* sort along the principal axis of s */
  calc_axis (idx, axis);
  sort_along_axis (axis, s->first, s->num);

  max[0] = s->u[0] * s->weight;
  max[1] = s->u[1] * s->weight;
  max[2] = s->u[2] * s->weight;

  first = colors + s->first;
  last = first + s->num;

  xi = 0;
  opt_num = 0;
  opt[0] = opt[1] = opt[2] = 0;
  value = 0;
  acc_num = 0;
  acc[0] = acc[1] = acc[2] = 0;

  c = first;
  while (1) {
    acc[0] += c->num * c->c[0];
    acc[1] += c->num * c->c[1];
    acc[2] += c->num * c->c[2];
    acc_num += c->num;
    c++;
    if (c == last) break;
    val_i = ((acc[0]*acc[0] + acc[1]*acc[1] + acc[2]*acc[2]) / acc_num
	     + (  (max[0] - acc[0])*(max[0] - acc[0])
		+ (max[1] - acc[1])*(max[1] - acc[1])
		+ (max[2] - acc[2])*(max[2] - acc[2]) ) 
	     / (s->weight - acc_num));
    if (value < val_i) {
      value = val_i;
      xi = c - first;
      opt_num = acc_num;
      opt[0] = acc[0]; opt[1] = acc[1]; opt[2] = acc[2];
    }
  }		

  c1->first = s->first;
  c1->num = xi; 
  c1->weight = opt_num;
  
  c1->u[0] = opt[0] / c1->weight;
  c1->u[1] = opt[1] / c1->weight;
  c1->u[2] = opt[2] / c1->weight;
  
  c2->first = c1->first + c1->num;
  c2->num = s->num - c1->num;
  c2->weight = s->weight - c1->weight;
  
  c2->u[0] = (max[0] - opt[0]) / c2->weight;
  c2->u[1] = (max[1] - opt[1]) / c2->weight;
  c2->u[2] = (max[2] - opt[2]) / c2->weight;

  if (vals.kmeans) {
    /* adaptive 2-clustering */	
    it = 0;
    ofs = 0; /* avoid compiler warning */
    convergence = 0;
    do {
      it++; 
      if (it == vals.max_iterations) { break; }
      c = first;
      while (c != last) {
	if (convergence == 0) {  	
	  /* find the plane normal of the separating plane */
	  normal[0] = c2->u[0] - c1->u[0];
	  normal[1] = c2->u[1] - c1->u[1];
	  normal[2] = c2->u[2] - c1->u[2];
	  
	  /* find the offset of the halfway vector between the centroids */
	  ofs = (  (c1->u[0] + c2->u[0])*normal[0]
		 + (c1->u[1] + c2->u[1])*normal[1]
		 + (c1->u[2] + c2->u[2])*normal[2] )/2;
	}
	cofs = c->c[0]*normal[0] + c->c[1]*normal[1] + c->c[2]*normal[2];
	if ((c < colors + c2->first) && (cofs > ofs) && (c1->num > 1)) {
	  convergence = 0;
	  /* update the samples */
	  c1->u[0] = (c1->weight*c1->u[0] - c->num*c->c[0])/(c1->weight - c->num);
	  c1->u[1] = (c1->weight*c1->u[1] - c->num*c->c[1])/(c1->weight - c->num);
	  c1->u[2] = (c1->weight*c1->u[2] - c->num*c->c[2])/(c1->weight - c->num);
	  c2->u[0] = (c2->weight*c2->u[0] + c->num*c->c[0])/(c2->weight + c->num);
	  c2->u[1] = (c2->weight*c2->u[1] + c->num*c->c[1])/(c2->weight + c->num);
	  c2->u[2] = (c2->weight*c2->u[2] + c->num*c->c[2])/(c2->weight + c->num);
	  c1->weight -= c->num;
	  c2->weight += c->num;
	  /* move c from c1 to c2 */
	  c1->num--;
	  c2->first--; c2->num++;
	  ctmp = *c; *c = colors[c2->first]; colors[c2->first] = ctmp;
	  /* do NOT increase c, we still have to check that color */
	} else if ((c >= colors + c2->first) && (cofs < ofs) && (c2->num > 1)) {
	  convergence = 0;
	  /* update the samples */
	  c1->u[0] = (c1->weight*c1->u[0] + c->num*c->c[0])/(c1->weight + c->num);
	  c1->u[1] = (c1->weight*c1->u[1] + c->num*c->c[1])/(c1->weight + c->num);
	  c1->u[2] = (c1->weight*c1->u[2] + c->num*c->c[2])/(c1->weight + c->num);
	  c2->u[0] = (c2->weight*c2->u[0] - c->num*c->c[0])/(c2->weight - c->num);
	  c2->u[1] = (c2->weight*c2->u[1] - c->num*c->c[1])/(c2->weight - c->num);
	  c2->u[2] = (c2->weight*c2->u[2] - c->num*c->c[2])/(c2->weight - c->num);
	  c1->weight += c->num;
	  c2->weight -= c->num;
	  /* move c from c2 to c1 */	
	  ctmp = *c; *c = colors[c2->first]; colors[c2->first] = ctmp;
	  c1->num++;
	  c2->first++; c2->num--;
	  /* do NOT increase c, we still have to check that color */	  
	} else { c++; convergence++; } /* this point is ok or can't be moved */
      } /* loop over all colors */
    } while (convergence < s->num); /* until cycle w/o move completed */
  } /* if kmeans allowed */

  calc_error (s->c1);
  calc_error (s->c2);
  s->gain = s->error - c1->error - c2->error;
}

/********************************************************************/
/* splitting along the principal axis                               */
/********************************************************************/

#define Wrs(a,r,s) (W_rs[((r)<<1)+(s)])[a]
#define W2(a) (Wrs(a,0,0) + Wrs(a,1,1) + Wrs(a,2,2))
#define EEE(a,b) (   W2(b) - W2(a) \
    - (  (W1[0][b] - W1[0][a])*(W1[0][b] - W1[0][a]) \
       + (W1[1][b] - W1[1][a])*(W1[1][b] - W1[1][a]) \
       + (W1[2][b] - W1[2][a])*(W1[2][b] - W1[2][a]) ) / (W0[b] - W0[a]) )
#define COV(a,b,r,s) ( (Wrs(b,r,s) - Wrs(a,r,s)) / (W0[b] - W0[a]) \
  - ((W1[r][b] - W1[r][a])*(W1[s][b] - W1[s][a])) / \
    ((W0[b] - W0[a])*(W0[b] - W0[a])) )

static int principal_split ()
{
  int i, j, k, go_on, princ, princ2, biased, add;
  int t, tt, cut, n, **L, *q, *W0;
  float *E, *W1[3], *W_rs[7], et, e;
  float deviation, Cov[3][3], U[3][3], lambda[3];
  float axis[3], bias, step, key;
  color *last, *c, ctmp;
  float *normal[3], *ofs, cofs0, cofs1;
  int convergence, it;

  k = 1;
  calc_centroid (0);
  bias = calc_axis (0, axis);
  biased = (bias > vals.min_bias);
  if (biased) {
    sort_along_axis (axis, 0, max_color);

    /* room for optimal principal quantizer */
    q = (int *) malloc ((vals.K + 1) * sizeof (int));  
    q[0] = 0;
    
    /* calculate the cumulative moments */
    W0 = (int *) malloc ((vals.resolution + 1) * sizeof (int));
    W1[0]  = (float *) malloc ((vals.resolution + 1) * sizeof (float));
    W1[1]  = (float *) malloc ((vals.resolution + 1) * sizeof (float));
    W1[2]  = (float *) malloc ((vals.resolution + 1) * sizeof (float));
    W_rs[0] = (float *) malloc ((vals.resolution + 1) * sizeof (float));
    W_rs[1] = (float *) malloc ((vals.resolution + 1) * sizeof (float));
    W_rs[2] = (float *) malloc ((vals.resolution + 1) * sizeof (float));
    W_rs[3] = (float *) malloc ((vals.resolution + 1) * sizeof (float));
    W_rs[4] = (float *) malloc ((vals.resolution + 1) * sizeof (float));
    W_rs[6] = (float *) malloc ((vals.resolution + 1) * sizeof (float));
    W0[0] = 0;
    W1[0][0] = W1[1][0] = W1[2][0] = 0;
    Wrs(0,0,0) = Wrs(0,0,1) = Wrs(0,0,2) = 0;
    Wrs(0,1,1) = Wrs(0,1,2) = Wrs(0,2,2) = 0;
    c = colors;
    last = colors + max_color;
    step = (colors[max_color-1].key - c->key) / (vals.resolution - 0.5);
    key = c->key - 0.25*step;
    for (i = 1; i <= vals.resolution; i++) {
      W0[i] = W0[i-1]; 
      W1[0][i] = W1[0][i-1];
      W1[1][i] = W1[1][i-1];
      W1[2][i] = W1[2][i-1];
      Wrs(i,0,0) = Wrs(i-1,0,0); 
      Wrs(i,0,1) = Wrs(i-1,0,1); 
      Wrs(i,0,2) = Wrs(i-1,0,2);
      Wrs(i,1,1) = Wrs(i-1,1,1);
      Wrs(i,1,2) = Wrs(i-1,1,2);
      Wrs(i,2,2) = Wrs(i-1,2,2);
      key += step;
      while (c->key <= key) {
	W0[i] += c->num; 
	W1[0][i] += c->num * (c->c[0]);
	W1[1][i] += c->num * (c->c[1]);
	W1[2][i] += c->num * (c->c[2]);
	Wrs(i,0,0) += c->num * (c->c[0] * c->c[0]); 
	Wrs(i,0,1) += c->num * (c->c[0] * c->c[1]); 
	Wrs(i,0,2) += c->num * (c->c[0] * c->c[2]);
	Wrs(i,1,1) += c->num * (c->c[1] * c->c[1]); 
	Wrs(i,1,2) += c->num * (c->c[1] * c->c[2]); 
	Wrs(i,2,2) += c->num * (c->c[2] * c->c[2]);     
	c++;
	if (c == last) break;
      }
    }

    /* initialize the deviations */
    E = (float *) malloc ((vals.resolution + 1) * sizeof (float));
    E[0] = 0;
    for (i = 1; i <= vals.resolution; i++) { E[i] = EEE(0,i); }
    
    /* Initialize the L's. 
       We allocate L[][] incrementally, since we don't know the 
       length of the optimal principal quantizer in advance. */
    L = (int **) malloc ((vals.K + 1) * sizeof (int *));
    
    /* dynamic programming */
    add = 0; /* counter for additional smoothness splits */
    do {
      gimp_progress_update((double) k / (double) vals.K);
#ifdef PROGRESS_MESSAGES
      printf ("reaching %d of %d colors\n", k, vals.K);
#endif      
      k++;
      L[k] = (int *) malloc ((vals.resolution + 1) * sizeof (int));
      L[k][k] = k - 1;
      for (n = vals.resolution; n > k; n--) {
	cut = n; 
	do { cut--; } while (W0[n] == W0[cut]); /* we want to cut something off! */
	e = E[cut] + EEE(cut,n);
 	t = cut;
	do {
	  tt = t;
	  do { t--; } while (W0[tt] == W0[t]); /* we want a different cut */
	  if (t < k - 1) { break; } 
	  et = EEE(t,n);
	  if (e < et) { break; } 
	  et += E[t];
	  if (et < e) { cut = t; e = et; } 
	} while (1); 
	L[k][n] = cut; E[n] = e;
      }
      
      /* determine the k-1 optimal splitting points */      
      q[k] = vals.resolution;
      for (j = k - 1; j >= 1; j--) {
	q[j] = L[j+1][q[j+1]];
      }
      
      go_on = 0;
      if (k < vals.K) {
	if (biased) {
	  biased = 0;
	  /* calc the axes, check the bias */
	  for (j = 0; j < k; j++) {
	    Cov[0][0] = COV(q[j],q[j+1],0,0);
	    Cov[1][1] = COV(q[j],q[j+1],1,1);
	    Cov[2][2] = COV(q[j],q[j+1],2,2);     
	    Cov[0][1] = Cov[1][0] = COV(q[j],q[j+1],0,1);
	    Cov[0][2] = Cov[2][0] = COV(q[j],q[j+1],0,2);
	    Cov[1][2] = Cov[2][1] = COV(q[j],q[j+1],1,2);      
	  
	    /* do a spectral decomposition of Cov */
	    spectral (Cov, U, lambda);
	    
	    /* find the largest eigenvalue, and the corr. eigenvector */
	    if (lambda[0] < 0) lambda[0] = - lambda[0];
	    if (lambda[1] < 0) lambda[1] = - lambda[1];
	    if (lambda[2] < 0) lambda[2] = - lambda[2];
	    
	    princ = 0;
	    if (lambda[1] > lambda[0]) princ = 1;
	    if (lambda[2] > lambda[princ]) princ = 2;
	    princ2 = (princ+1) % 3; 
	    if (lambda[(princ+2) % 3] > lambda[princ2]) princ2 = (princ+2) % 3; 
	    
	    deviation = axis[0]*U[0][princ]+axis[1]*U[1][princ]+axis[2]*U[2][princ];
	    if (deviation < 0) deviation = -deviation; 
	    
	    if ((lambda[princ] > vals.min_bias*lambda[princ2]) && 
		(deviation > vals.max_deviation)) {
	      biased = 1;  /* look for more splits */
	      break;  /* no need to look at more axes */
	    }
	  } /* check bias loop */
	} /* if was biased */
	if (biased) {
	  go_on = 1;
	} else {
	  add++;
	  go_on = (add < vals.additional);
	}
      } /* if more leafs needed */
    } while (go_on);   /* still biased ? */
      

    /* free storage for dynamic programming */
    for (j = 2; j <= k; j++) { free (L[j]); }
    free (L); 
    free (E); 
    free (W0);
    free (W1[0]); free (W1[1]); free (W1[2]);
    free (W_rs[0]); free (W_rs[1]); free (W_rs[2]);
    free (W_rs[3]); free (W_rs[4]); free (W_rs[6]);
    
    /* splitting at the positions stored in q */
    c = colors; 
    key = c->key - 0.25*step;
    for (j = 0; j < k; j++) {
      samples[j].status = LEAF;   
      samples[j].first = c - colors;
      samples[j].num = 0;
      samples[j].weight = 0;
      samples[j].u[0] = 0;
      samples[j].u[1] = 0;
      samples[j].u[2] = 0;
      key += (q[j+1] - q[j]) * step; 
      while ((c!= last) && (c->key <= key)) {
	samples[j].num++;
	samples[j].weight += c->num;
	samples[j].u[0] += c->num * c->c[0];
	samples[j].u[1] += c->num * c->c[1];
	samples[j].u[2] += c->num * c->c[2];
	c++;
      }
      
      samples[j].u[0] /= samples[j].weight;
      samples[j].u[1] /= samples[j].weight;
      samples[j].u[2] /= samples[j].weight;
    }
    free (q);
    
    if (vals.kmeans) {
      /* adaptive k-clustering */
      normal[0] = (float *) malloc ((k + 1) * sizeof (float));
      normal[1] = (float *) malloc ((k + 1) * sizeof (float));
      normal[2] = (float *) malloc ((k + 1) * sizeof (float));
      ofs = (float *) malloc ((k + 1) * sizeof (float));
      
      for (i = 1; i < k; i++) {
	normal[0][i] = samples[i].u[0] - samples[i-1].u[0];
	normal[1][i] = samples[i].u[1] - samples[i-1].u[1];
	normal[2][i] = samples[i].u[2] - samples[i-1].u[2];
	ofs[i] = (  (samples[i-1].u[0] + samples[i].u[0])*normal[0][i]	
		  + (samples[i-1].u[1] + samples[i].u[1])*normal[1][i]
		  + (samples[i-1].u[2] + samples[i].u[2])*normal[2][i] )/2;
      }
      ofs[0] = -1; ofs[k] = 1;
      normal[0][0] = normal[1][0] = normal[2][0] = 0;
      normal[0][k] = normal[1][k] = normal[2][k] = 0;
      
      it = 0;
      convergence = 0;
      do {
	it++; 
	if (it == vals.max_iterations) { break; }
	i = 0; c = colors;
	while (c != last) {
	  if (samples[i].num > 1) {
	    /* this sample can get smaller */
	    cofs0=c->c[0]*normal[0][i] + c->c[1]*normal[1][i] + c->c[2]*normal[2][i];
	    cofs1=c->c[0]*normal[0][i+1]+c->c[1]*normal[1][i+1]+c->c[2]*normal[2][i+1];
	    if ((cofs0 < ofs[i]) || (cofs1 > ofs[i+1])) {
	      convergence = 0;
	      if (cofs0 < ofs[i]) {
		/* update samples i and i - 1 */
		samples[i].u[0] =((samples[i].weight*samples[i].u[0]-c->num*c->c[0])
				  /(samples[i].weight - c->num));
		samples[i].u[1] =((samples[i].weight*samples[i].u[1]-c->num*c->c[1])
				  /(samples[i].weight - c->num));
		samples[i].u[2] =((samples[i].weight*samples[i].u[2]-c->num*c->c[2])
				  /(samples[i].weight - c->num));
		samples[i-1].u[0]=((samples[i-1].weight*samples[i-1].u[0]+c->num*c->c[0])
				   /(samples[i-1].weight + c->num));
		samples[i-1].u[1]=((samples[i-1].weight*samples[i-1].u[1]+c->num*c->c[1])
				   /(samples[i-1].weight + c->num));
		samples[i-1].u[2]=((samples[i-1].weight*samples[i-1].u[2]+c->num*c->c[2])
				   /(samples[i-1].weight + c->num));
		samples[i].weight -= c->num;
		samples[i-1].weight += c->num;
		/* move c to sample i - 1 */
		ctmp=*c;*c=colors[samples[i].first];colors[samples[i].first]=ctmp;
		samples[i-1].num++; samples[i].num--; samples[i].first++; 
	      } else {
		/* update samples i and i + 1 */
		samples[i].u[0] = (  (samples[i].weight*samples[i].u[0] - c->num*c->c[0])
				   / (samples[i].weight - c->num));
		samples[i].u[1] = (  (samples[i].weight*samples[i].u[1] - c->num*c->c[1])
				   / (samples[i].weight - c->num));
		samples[i].u[2] = (  (samples[i].weight*samples[i].u[2] - c->num*c->c[2])
				   / (samples[i].weight - c->num));
		samples[i+1].u[0]=((samples[i+1].weight*samples[i+1].u[0]+c->num*c->c[0])
				   /(samples[i+1].weight + c->num));
		samples[i+1].u[1]=((samples[i+1].weight*samples[i+1].u[1]+c->num*c->c[1])
				   /(samples[i+1].weight + c->num));
		samples[i+1].u[2]=((samples[i+1].weight*samples[i+1].u[2]+c->num*c->c[2])
				   /(samples[i+1].weight + c->num));
		samples[i].weight -= c->num;
		samples[i+1].weight += c->num;
		/* move c to sample i + 1 */
		samples[i].num--; samples[i+1].num++; samples[i+1].first--; 
		ctmp=*c; *c=colors[samples[i+1].first]; colors[samples[i+1].first]=ctmp;
	      }
	      if (i > 0) {
		/* update normal and offset i */
		normal[0][i] = samples[i].u[0] - samples[i-1].u[0];
		normal[1][i] = samples[i].u[1] - samples[i-1].u[1];
		normal[2][i] = samples[i].u[2] - samples[i-1].u[2];
		ofs[i] = (  (samples[i-1].u[0] + samples[i].u[0])*normal[0][i]
			  + (samples[i-1].u[1] + samples[i].u[1])*normal[1][i]
			  + (samples[i-1].u[2] + samples[i].u[2])*normal[2][i] )/2;
	      }
	      if (i + 1 < k) {
		/* update normal and offset i + 1 */	  
		normal[0][i+1] = samples[i+1].u[0] - samples[i].u[0];
		normal[1][i+1] = samples[i+1].u[1] - samples[i].u[1];
		normal[2][i+1] = samples[i+1].u[2] - samples[i].u[2];
		ofs[i+1] = (  (samples[i].u[0] + samples[i+1].u[0])*normal[0][i+1]
			    + (samples[i].u[1] + samples[i+1].u[1])*normal[1][i+1]
			    + (samples[i].u[2] + samples[i+1].u[2])*normal[2][i+1] )/2;
	      }
	      /* do NOT increase c, we still have to check that color */
	      /* avoid the following test in the outer loop */
	      if (c < colors + samples[i].first) { i--; } 
	    } else { convergence++; c++; } /* c is ok */
	  } else { convergence++; c++; } /* single-color sample */
	  /* update i */
	  if (colors + samples[i].first + samples[i].num <= c) { i++; } 
	}
      } while (convergence < max_color);
      
      free (normal[0]);
      free (normal[1]);
      free (normal[2]);
      free (ofs);
    } /* if kmeans allowed */
  } /* if initially biased */  
  return k; /* return the number of leafs */
}

/******************************************************************/
/* main function                                                  */
/******************************************************************/

static void quant (GDrawable *drawable)
{
  int i, j, splitting, leafs;
  float gain;

  if (vals.luv) init_CIE ();

  gimp_tile_cache_ntiles (2 * (drawable->width / gimp_tile_width () + 1));

#ifdef PROGRESS_MESSAGES  
  printf ("Building histogram\n"); 
#endif
  build_histogram (drawable);

  gimp_progress_init("(2/3) Building colormap...");
  gimp_progress_update(0);

  max_sample = 4 * vals.K; /* more than enough */
  samples = (sample *) malloc (max_sample * sizeof (sample));
  
  /* create the root sample */	
  samples[0].status = LEAF;
  samples[0].first = 0;
  samples[0].num = max_color;
  leafs = 1;
  /* init the other samples */
  for (i = 1; i < max_sample; i++) { samples[i].status = UNUSED; }
  
  if (vals.principal) {
#ifdef PROGRESS_MESSAGES  
    printf("Principal quantization\n");
#endif
    /* do the split along the principal axis */ 
    gimp_progress_init("(2/3) Principal quantisation...");
    leafs = principal_split ();
    gimp_progress_init("(2/3) Building colormap...");
  } else {
    calc_centroid (0);
  }
  /* The gimp_progress_update was done in principal_split(). */
  for (i = 0; i < leafs; i++) {
    calc_error (i);
    binary_split (i);
  }

  if (leafs < vals.K) {
  /* additional split loop */
#ifdef PROGRESS_MESSAGES  
    if (leafs < vals.K) printf("Binary splitting\n");
#endif
  while (1) {

#ifdef PROGRESS_MESSAGES  
    printf("reaching %d of %d colors\n", leafs, vals.K);
#endif    
    gimp_progress_update((double) leafs / (double) vals.K);

    /* find splitting sample (we implement the max-gain splitting strategy) */
    splitting = -1; 
    gain = 0; 
    for (i = 0; i < max_sample; i++) {
      if ((samples[i].status == LEAF) && (samples[i].gain > gain)) {
	splitting = i;
	gain = samples[i].gain;
      }
    }
    if (splitting < 0) { break; } /* cannot split any more */
    
    /* split the selected sample */
    samples[splitting].status = UNUSED;
    samples[samples[splitting].c1].status = LEAF;
    samples[samples[splitting].c2].status = LEAF; 
    leafs++;

    if (leafs >= vals.K) { break; } /* enough leafs found */
      
    binary_split (samples[splitting].c1);
    binary_split (samples[splitting].c2);
    
  }  /* end of splitting loop */
  }  /* end of "avoid nasty bug" if: 
      * the '>=' six lines above was a '==' ...) */

#ifdef PROGRESS_MESSAGES
  printf("reaching %d of %d colors\n", leafs, vals.K);
#endif

  free (colors);

#ifdef PROGRESS_MESSAGES
  printf ("Nearest-neighbour mapping\n");
#endif

  /* bring the leafs to the top of the samples array;
     transform their centroids to image space */
  j = max_sample;
  for (i = 0; i < leafs; i++) {
    if (samples[i].status < LEAF) {
      do {
	j--;
	if (samples[j].status >= LEAF) {
	  samples[i] = samples[j];
	  samples[j].status = UNUSED;
	  break;
	}
      } while (j > i);
    }
    goto_imagespace (samples[i].u, samples[i].ci);
  }
  map_hashed (drawable, samples, leafs);
  /* update the processed region */
  gimp_drawable_flush(drawable);
  gimp_drawable_merge_shadow(drawable->id, TRUE);
  gimp_drawable_update (drawable->id, 0, 0,
                        gimp_drawable_width(drawable->id),
                        gimp_drawable_height(drawable->id));

  free (samples);
}

</XMP></BODY></HTML>
