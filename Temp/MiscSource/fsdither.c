<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!-- saved from url=(0049)http://www.home.unix-ag.org/simon/gimp/fsdither.c -->
<HTML><HEAD>
<META content="text/html; charset=windows-1252" http-equiv=Content-Type>
<META content="MSHTML 5.00.2920.0" name=GENERATOR></HEAD>
<BODY><XMP>/*
 *  Floyd-Steinberg-Dither plug-in v0.8 by
 *  Simon Budig <Simon.Budig@unix-ag.org> 06.09.98
 *  Copyright (C) 1998 Simon Budig
 *
 * Plugin-structure based on the semi-flatten plugin by Adam D. Moss.
 * Dialog code based on the gauss_iir plugin by Spencer Kimball & Peter Mattis.
 *
 */

/*
 * A plugin for the GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * TODO:
 *     Use tile iteration instead of dumb row-walking. How is it useful
 *     to store the various errors?
 */



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#include <libgimp/gimp.h>

#define ENTRY_WIDTH 100

typedef struct
{
  gint red;
  gint green;
  gint blue;
  gint gray;
  gint alpha;
  gint shades;
} FSDValues;

/* Declare local functions.
 */
static void      query  (void);
static void      run    (char      *name,
			 int        nparams,
			 GParam    *param,
			 int       *nreturn_vals,
			 GParam   **return_vals);

static void      fsdither            (GDrawable  *drawable,
                                      guint* channels,
                                      gint shades);

static void      fsdither_render_row (const guchar *src_row,
		                      guchar *dest_row,
		                      gfloat *error,
		                      gfloat *nexterror,
		                      gint* channels,
				      gint shades,
		                      gint row_width,
		                      gint bytes);

static gint      fsdither_dialog (gint color, gint alpha);

static void      fsdither_close_callback  (GtkWidget *widget,
                                           gpointer   data);
static void      fsdither_ok_callback     (GtkWidget *widget,
                                           gpointer   data);
static void      fsdither_toggle_update   (GtkWidget *widget,
                                           gpointer   data);
static void      fsdither_entry_callback   (GtkWidget *widget,
                                           gpointer   data);


static FSDValues fsdvals = 
{
  TRUE,  /* RED */
  TRUE,  /* GREEN */
  TRUE,  /* BLUE */
  TRUE,  /* GRAY */
  TRUE,  /* ALPHA */
  2      /* No. of shades */
};

static gint dialog_ok = FALSE;


GPlugInInfo PLUG_IN_INFO =
{
  NULL,    /* init_proc */
  NULL,    /* quit_proc */
  query,   /* query_proc */
  run,     /* run_proc */
};


MAIN ()


static void
query ()
{
  static GParamDef args[] =
  {
    { PARAM_INT32, "run_mode", "Interactive, non-interactive" },
    { PARAM_IMAGE, "image", "Input image (unused)" },
    { PARAM_DRAWABLE, "drawable", "Input drawable" },
    { PARAM_INT32, "Shades", "Number of shades of gray (2-256)" },
    { PARAM_INT32, "Red", "Dither red channel if available (TRUE/FALSE)" },
    { PARAM_INT32, "Green", "Dither green channel if available (TRUE/FALSE)" },
    { PARAM_INT32, "Blue", "Dither blue channel if available (TRUE/FALSE)" },
    { PARAM_INT32, "Gray", "Dither gray channel if available (TRUE/FALSE)" },
    { PARAM_INT32, "Alpha", "Dither alpha channel if available (TRUE/FALSE)" },
  };
  static GParamDef *return_vals = NULL;
  static int nargs = sizeof (args) / sizeof (args[0]);
  static int nreturn_vals = 0;

  gimp_install_procedure ("plug_in_fsdither",
			  "Dithers a channel to B/W. Esp. useful for creating B/W-Images or dithering the alpha-channel.",
			  "This Plugin dithers a channel to B/W. Esp. useful for creating B/W-Images or dithering the alpha-channel.",
			  "Simon Budig <Simon.Budig@unix-ag.org>",
			  "Simon Budig <Simon.Budig@unix-ag.org>",
			  "6. September 1998",
			  "<Image>/Filters/Colors/FS-Dither",
			  "RGB*, GRAY*",
			  PROC_PLUG_IN,
			  nargs, nreturn_vals,
			  args, return_vals);
}


static void
run (char    *name,
     int      nparams,
     GParam  *param,
     int     *nreturn_vals,
     GParam **return_vals)
{
  static GParam values[1];
  GDrawable *drawable;
  GRunModeType run_mode;
  gint32 image_ID;
  GStatusType status = STATUS_SUCCESS;
  gint *channels,channel, todo;

  run_mode = param[0].data.d_int32;
  *nreturn_vals = 1;
  *return_vals = values;

  values[0].type = PARAM_STATUS;
  values[0].data.d_status = status;

  /*  Get the specified drawable  */
  drawable = gimp_drawable_get (param[2].data.d_drawable);
  image_ID = param[1].data.d_image;

  /* Set presets according to drawabla type */
  switch (gimp_drawable_type (drawable->id)) {
    case RGBA_IMAGE:
    case GRAYA_IMAGE:
      fsdvals.red = fsdvals.green = fsdvals.blue = fsdvals.gray = FALSE;
      fsdvals.alpha = TRUE;
      break;
    case RGB_IMAGE:
      fsdvals.red = fsdvals.green = fsdvals.blue = TRUE;
      fsdvals.gray = fsdvals.alpha = FALSE;
      break;
    case GRAY_IMAGE:
      fsdvals.red = fsdvals.green = fsdvals.blue = fsdvals.alpha = FALSE;
      fsdvals.gray = TRUE;
      break;
    default:
      fsdvals.red = fsdvals.green = fsdvals.blue =
                            fsdvals.gray = fsdvals.alpha = FALSE;
      break;
  };

  /* Retrieve parameters according to Run-Mode */
  switch (run_mode) {
    case RUN_INTERACTIVE:
      gimp_get_data ("plug_in_fsdither", &fsdvals);
      if (! fsdither_dialog (gimp_drawable_color(drawable->id), gimp_drawable_has_alpha(drawable->id)))
        return;
      break;
    case RUN_NONINTERACTIVE:
      /*  Make sure all the arguments are there!  */
      if (nparams != 9)
        status = STATUS_CALLING_ERROR;
      if (status == STATUS_SUCCESS)
        {
          fsdvals.shades = (param[3].data.d_int32);
          fsdvals.red = (param[4].data.d_int32) ? TRUE : FALSE;
          fsdvals.green = (param[5].data.d_int32) ? TRUE : FALSE;
          fsdvals.blue = (param[6].data.d_int32) ? TRUE : FALSE;
          fsdvals.gray = (param[7].data.d_int32) ? TRUE : FALSE;
          fsdvals.alpha = (param[8].data.d_int32) ? TRUE : FALSE;
        }
      break;

    case RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      gimp_get_data ("plug_in_fsdither", &fsdvals);
      break;

    default:
      break;
  }

  if (status == STATUS_SUCCESS)
    {
      channels = g_malloc(gimp_drawable_bpp(drawable->id) * sizeof(gint));
      for (channel=0; channel < gimp_drawable_bpp(drawable->id); channel++)
         channels[channel] = FALSE;

      /*  Specify the relevant byte-positions  */
      switch (gimp_drawable_type (drawable->id)) {
        case RGBA_IMAGE:
          channels[gimp_drawable_bpp(drawable->id) - 1] = fsdvals.alpha;
        case RGB_IMAGE:
          channels[0]=fsdvals.red;
          channels[1]=fsdvals.green;
          channels[2]=fsdvals.blue;
          break;
        case GRAYA_IMAGE:
          channels[gimp_drawable_bpp(drawable->id) - 1] = fsdvals.alpha;
        case GRAY_IMAGE:
          channels[0]=fsdvals.gray;
          break;
        default:
	  status = STATUS_EXECUTION_ERROR;
          break;
      }

      if (fsdvals.shades < 2) fsdvals.shades = 2;
      if (fsdvals.shades > 256) fsdvals.shades = 256;

      /* Check if there is something to do */
      todo = FALSE;
      for (channel=0; channel < gimp_drawable_bpp(drawable->id); channel++)
        todo |= channels [channel];

      /* Run the dithering */
      if (status == STATUS_SUCCESS && todo) {
        gimp_progress_init ("FS-Dither...");
        gimp_tile_cache_ntiles (2 * (drawable->width / gimp_tile_width () + 1));
        fsdither(drawable, channels, fsdvals.shades);
        if (run_mode != RUN_NONINTERACTIVE)
          gimp_displays_flush ();
        if (run_mode == RUN_INTERACTIVE)
            gimp_set_data ("plug_in_fsdither", &fsdvals, sizeof (FSDValues));

      }
      g_free(channels);
    }

  values[0].data.d_status = status;

  gimp_drawable_detach (drawable);

}

static gint fsdither_dialog (gint color, gint alpha) {
  GtkWidget *dlg;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *button;
  GtkWidget *toggle;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *hbox;
  gchar buffer[12];
  gchar **argv;
  gint argc;

  argc = 1;
  argv = g_new (gchar *, 1);
  argv[0] = g_strdup ("fsdither");

  gtk_init (&argc, &argv);
  gtk_rc_parse (gimp_gtkrc ());

  dlg = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dlg), "Floyd-Steinberg Dither");
  gtk_window_position (GTK_WINDOW (dlg), GTK_WIN_POS_MOUSE);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
                      (GtkSignalFunc) fsdither_close_callback,
                      NULL);

  /*  Action area  */
  button = gtk_button_new_with_label ("OK");
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      (GtkSignalFunc) fsdither_ok_callback,
                      dlg);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), button, TRUE, TRUE, 0);
  gtk_widget_grab_default (button);
  gtk_widget_show (button);

  button = gtk_button_new_with_label ("Cancel");
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                             (GtkSignalFunc) gtk_widget_destroy,
                             GTK_OBJECT (dlg));
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area), button, TRUE, TRUE, 0);
  gtk_widget_show (button);

  /*  parameter settings  */
  frame = gtk_frame_new ("Channels to dither:");
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_IN);
  gtk_container_border_width (GTK_CONTAINER (frame), 10);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), frame, TRUE, TRUE, 0);
  vbox = gtk_vbox_new (FALSE, 5);
  gtk_container_border_width (GTK_CONTAINER (vbox), 10);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  if (color) {
    toggle = gtk_check_button_new_with_label ("Red");
    gtk_box_pack_start (GTK_BOX (vbox), toggle, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (toggle), "toggled",
                        (GtkSignalFunc) fsdither_toggle_update,
                        &fsdvals.red);
    gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (toggle), fsdvals.red);
    gtk_widget_show (toggle);

    toggle = gtk_check_button_new_with_label ("Green");
    gtk_box_pack_start (GTK_BOX (vbox), toggle, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (toggle), "toggled",
                        (GtkSignalFunc) fsdither_toggle_update,
                        &fsdvals.green);
    gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (toggle), fsdvals.green);
    gtk_widget_show (toggle);
  
    toggle = gtk_check_button_new_with_label ("Blue");
    gtk_box_pack_start (GTK_BOX (vbox), toggle, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (toggle), "toggled",
                        (GtkSignalFunc) fsdither_toggle_update,
                        &fsdvals.blue);
    gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (toggle), fsdvals.blue);
    gtk_widget_show (toggle);
  } else {
    toggle = gtk_check_button_new_with_label ("Gray");
    gtk_box_pack_start (GTK_BOX (vbox), toggle, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (toggle), "toggled",
                        (GtkSignalFunc) fsdither_toggle_update,
                        &fsdvals.gray);
    gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (toggle), fsdvals.gray);
    gtk_widget_show (toggle);
  }
  if (alpha) {
    toggle = gtk_check_button_new_with_label ("Alpha");
    gtk_box_pack_start (GTK_BOX (vbox), toggle, TRUE, TRUE, 0);
    gtk_signal_connect (GTK_OBJECT (toggle), "toggled",
                        (GtkSignalFunc) fsdither_toggle_update,
                        &fsdvals.alpha);
    gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (toggle), fsdvals.alpha);
    gtk_widget_show (toggle);
  }

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  label = gtk_label_new ("No. of Shades: ");
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, FALSE, 0);
  gtk_widget_show (label);

  entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
  gtk_widget_set_usize (entry, ENTRY_WIDTH, 0);
  sprintf (buffer, "%i", fsdvals.shades);
  gtk_entry_set_text (GTK_ENTRY (entry), buffer);
  gtk_signal_connect (GTK_OBJECT (entry), "changed",
                      (GtkSignalFunc) fsdither_entry_callback,
                      NULL);
  gtk_widget_show (entry);

  gtk_widget_show (hbox);

  gtk_widget_show (vbox);
  gtk_widget_show (frame);
  gtk_widget_show (dlg);

  gtk_main ();
  gdk_flush ();
  return dialog_ok;
}

/*  FSDither interface callback functions  */

static void
fsdither_close_callback (GtkWidget *widget,
                         gpointer   data)
{
  gtk_main_quit ();
}

static void
fsdither_ok_callback (GtkWidget *widget,
                      gpointer   data)
{
  dialog_ok = TRUE;
  gtk_widget_destroy (GTK_WIDGET (data));
}

static void
fsdither_toggle_update (GtkWidget *widget,
                        gpointer   data)
{
  gint *toggle_val;

  toggle_val = (gint *) data;

  if (GTK_TOGGLE_BUTTON (widget)->active)
    *toggle_val = TRUE;
  else
    *toggle_val = FALSE;
}

static void
fsdither_entry_callback (GtkWidget *widget,
                      gpointer   data)
{
  fsdvals.shades = atoi (gtk_entry_get_text (GTK_ENTRY (widget)));
  if (fsdvals.shades < 2)
    fsdvals.shades = 2;
  if (fsdvals.shades > 256)
    fsdvals.shades = 256;
}

/***************************************************/


/* 
 * The dither function: Floyd Steinberg Dithering.
 * The value of an channel will be changed to match one of the given
 * (No. of Shades) values. The error will be distributed according to the
 * following table:
 *
 * Direction: ----->
 *
 *    +------+------+------+
 *    |      |curr. |      |
 *    |      |Pixel | 7/16 |
 *    |      |      |      |
 *    +------+------+------+
 *    |      |      |      |
 *    | 3/16 | 5/16 | 1/16 |
 *    |      |      |      |
 *    +------+------+------+
 *
 * A slightly better result is reached by changing the direction every
 * second row.
 */

static void
fsdither_render_row (const guchar *src_row,
		  guchar *dest_row,
		  gfloat *error,
		  gfloat *nexterror,
		  gint *channels,
                  gint shades,
		  gint row_width,
		  gint bytes)
{
  gint col;
  gint channel;
  static gint direction = 1;
  gint start, end;

  gfloat newval, factor;
  gfloat cerror;

  for (col = 0; col < row_width*bytes; col++) nexterror[col] = 0;

  if (direction==1) {
    start=0; end=row_width;
  } else {
    direction = -1;
    start=row_width-1; end=-1;
  }

  factor = (float) (shades-1)/ (float) 255;

  for (channel = 0; channel < bytes; channel++) {
    if (channels[channel]) {
      /* If channel has to be dithered */
      for (col = start; col != end ; col+=direction)
        {
          newval = src_row[col*bytes+channel] + error[col*bytes+channel];
          newval *= factor;
          newval = floor (newval + 0.5);
          newval /= factor;
          if (newval > 255) newval = 255 ;
          if (newval < 0) newval = 0 ;

          dest_row[col*bytes+channel] = (guint) (newval + 0.5);

          cerror = src_row[col*bytes+channel] + error[col*bytes+channel] 
                     - dest_row[col*bytes+channel];
  
          nexterror[col*bytes+channel] += cerror * 5/16;
          if (col+direction >= 0 && col+direction < row_width) {
            error[(col+direction)*bytes+channel] += cerror * 7/16;
            nexterror[(col+direction)*bytes+channel] += cerror * 1/16;
          }
          if (col-direction >= 0 && col-direction < row_width)
            nexterror[(col-direction)*bytes+channel] += cerror * 3/16;
        }
    } else {
      /* Else copy pixels unmodified */
      for (col = 0; col < row_width; col++)
        dest_row[col*bytes+channel]=src_row[col*bytes+channel];
    }
  }
  direction = -direction;  /* Render next row in other direction */
}

static void
fsdither (GDrawable *drawable, guint* channels, gint shades)
{
  GPixelRgn srcPR, destPR;
  gint width, height;
  gint bytes;
  guchar *src_row;
  guchar *dest_row;
  gfloat *error, *nexterror, *tmp;
  gint row;
  gint x1, y1, x2, y2;


  /* Get the input area. This is the bounding box of the selection in
   *  the image (or the entire image if there is no selection). Only
   *  operating on the input area is simply an optimization. It doesn't
   *  need to be done for correct operation. (It simply makes it go
   *  faster, since fewer pixels need to be operated on).
   */
  gimp_drawable_mask_bounds (drawable->id, &x1, &y1, &x2, &y2);

  /* Get the size of the input image. (This will/must be the same
   *  as the size of the output image.
   */
  width = drawable->width;
  height = drawable->height;
  bytes = drawable->bpp;

  /*  allocate row/error buffers  */
  src_row = (guchar *) g_malloc ((x2 - x1) * bytes);
  dest_row = (guchar *) g_malloc ((x2 - x1) * bytes);
  error = (gfloat *) g_malloc ((x2 - x1) * bytes * sizeof (gfloat));
  nexterror = (gfloat *) g_malloc ((x2 - x1) * bytes * sizeof (gfloat));

  /*  initialize the error buffers  */
  for (row = 0; row < (x2-x1)*bytes; row++) error[row] = nexterror[row] = 0;

  /*  initialize the pixel regions  */
  gimp_pixel_rgn_init (&srcPR, drawable, 0, 0, width, height, FALSE, FALSE);
  gimp_pixel_rgn_init (&destPR, drawable, 0, 0, width, height, TRUE, TRUE);


  for (row = y1; row < y2; row++)
    {
      gimp_pixel_rgn_get_row (&srcPR, src_row, x1, row, (x2 - x1));

      fsdither_render_row (src_row,
			dest_row,
			error,
			nexterror,
			channels,
                        shades,
			(x2 - x1),
			bytes
			);

      /*  store the dest  */
      gimp_pixel_rgn_set_row (&destPR, dest_row, x1, row, (x2 - x1));

      /* swap error/nexerror */
      tmp=error; error=nexterror; nexterror=tmp;

      if ((row % 10) == 0)
	gimp_progress_update ((double) row / (double) (y2 - y1));
    }

  /*  update the processed region  */
  gimp_drawable_flush (drawable);
  gimp_drawable_merge_shadow (drawable->id, TRUE);
  gimp_drawable_update (drawable->id, x1, y1, (x2 - x1), (y2 - y1));

  g_free (src_row);
  g_free (dest_row);
  g_free (error);
  g_free (nexterror);
}

</XMP></BODY></HTML>
