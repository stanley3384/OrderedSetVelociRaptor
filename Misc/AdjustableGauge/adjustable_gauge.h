
/*

    For use with adjustable_gauge.c. Look in adjustable_gauge_main.c for more information.

    C. Eric Cashon

*/

#ifndef __ADJUSTABLE_GAUGE_H__
#define __ADJUSTABLE_GAUGE_H__

//adjustable_gauge.h included after #include<gtk/gtk.h> in adjustable_gauge.c. Everything is in gtk.

G_BEGIN_DECLS

#define ADJUSTABLE_GAUGE_TYPE (adjustable_gauge_get_type())
#define ADJUSTABLE_GAUGE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ADJUSTABLE_GAUGE_TYPE, AdjustableGauge))
#define ADJUSTABLE_GAUGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ADJUSTABLE_GAUGE_TYPE, AdjustableGaugeClass))
#define IS_ADJUSTABLE_GAUGE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ADJUSTABLE_GAUGE_TYPE))
#define IS_ADJUSTABLE_GAUGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ADJUSTABLE_GAUGE_TYPE)
#define ADJUSTABLE_GAUGE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ADJUSTABLE_GAUGE_TYPE, AdjustableGaugeClass))

typedef struct _AdjustableGauge AdjustableGauge;
typedef struct _AdjustableGaugeClass AdjustableGaugeClass;
struct _AdjustableGauge
{
  GtkDrawingArea da;
};
struct _AdjustableGaugeClass
{
  GtkDrawingAreaClass parent_class;
};

//Public functions.
GType adjustable_gauge_get_type(void) G_GNUC_CONST;
GtkWidget* adjustable_gauge_new();

void adjustable_gauge_set_first_cutoff(AdjustableGauge *da, gdouble first_cutoff);
gdouble adjustable_gauge_get_first_cutoff(AdjustableGauge *da);
void adjustable_gauge_set_second_cutoff(AdjustableGauge *da, gdouble second_cutoff);
gdouble adjustable_gauge_get_second_cutoff(AdjustableGauge *da);
void adjustable_gauge_set_needle(AdjustableGauge *da, gdouble needle);
gdouble adjustable_gauge_get_needle(AdjustableGauge *da);
void adjustable_gauge_set_scale_bottom(AdjustableGauge *da, gdouble scale_bottom);
gdouble adjustable_gauge_get_scale_bottom(AdjustableGauge *da);
void adjustable_gauge_set_scale_top(AdjustableGauge *da, gdouble scale_top);
gdouble adjustable_gauge_get_scale_top(AdjustableGauge *da);

G_END_DECLS

#endif 
