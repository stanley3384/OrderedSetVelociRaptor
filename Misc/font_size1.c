
/*
gcc -Wall font_size1.c -o font_size1 `pkg-config --cflags --libs gtk+-3.0`

Some test code to look at some font properties with Pango and GTK+. 

Prints out
  Monospace
  Size 10 ApproximateWidth 8192 Ascent 13312 Descent 4096 Ascent+Descent 17408
  LMRoman10
  Size 10 ApproximateWidth 6190 Ascent 16384 Descent 4096 Ascent+Descent 20480

Might have to change font names if they aren't available. 

*/

#include<gtk/gtk.h>

int main(int argc, char **argv)
 {
     gint font_size1=10;
     gint font_size2=10;
     gint size1, size2, font_ascent1, font_descent1, font_ascent2, font_descent2, font_combine1, font_combine2, appr_width1, appr_width2;

     gtk_init(&argc, &argv);
     GtkWidget *textview=gtk_text_view_new();

     PangoContext *context1=gtk_widget_get_pango_context(GTK_WIDGET(textview));

     PangoFontDescription * mono_desc=pango_font_description_new();
     PangoFontDescription * roman_desc=pango_font_description_new();

     pango_font_description_set_family(mono_desc, "Monospace");
     pango_font_description_set_family(roman_desc, "LMRoman10");

     pango_font_description_set_size(mono_desc, font_size1*PANGO_SCALE);
     pango_font_description_set_size(roman_desc, font_size2*PANGO_SCALE);

     PangoFont *font1=pango_context_load_font(context1, mono_desc);
     PangoFont *font2=pango_context_load_font(context1, roman_desc);

     PangoFontMetrics *metrics1=pango_font_get_metrics(font1, NULL);
     PangoFontMetrics *metrics2=pango_font_get_metrics(font2, NULL);

     appr_width1=pango_font_metrics_get_approximate_char_width(metrics1);
     appr_width2=pango_font_metrics_get_approximate_char_width(metrics2);

     size1=pango_font_description_get_size(mono_desc)/PANGO_SCALE;
     size2=pango_font_description_get_size(roman_desc)/PANGO_SCALE;

     font_ascent1=pango_font_metrics_get_ascent(metrics1);
     font_descent1=pango_font_metrics_get_descent(metrics1);

     font_ascent2=pango_font_metrics_get_ascent(metrics2);
     font_descent2=pango_font_metrics_get_descent(metrics2);

     font_combine1=font_ascent1+font_descent1;
     font_combine2=font_ascent2+font_descent2;

     g_print("%s\n", pango_font_description_get_family(mono_desc));
     g_print("Size %i ApproximateWidth %i Ascent %i Descent %i Ascent+Descent %i\n", size1, appr_width1, font_ascent1, font_descent1, font_combine1);
     g_print("%s\n", pango_font_description_get_family(roman_desc));
     g_print("Size %i ApproximateWidth %i Ascent %i Descent %i Ascent+Descent %i\n", size2, appr_width2, font_ascent2, font_descent2, font_combine2);

     pango_font_description_free(mono_desc);
     pango_font_description_free(roman_desc);

     return 0;
  }
