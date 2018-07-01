#include "gst.h"

#include <gst/app/gstappsrc.h>

GMainLoop *main_loop = NULL;
void gstreamer_recieve_mainloop(void) {
  main_loop = g_main_loop_new(NULL, FALSE);

  g_main_loop_run(main_loop);
}

static gboolean gstreamer_recieve_bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
  switch (GST_MESSAGE_TYPE(msg)) {

  case GST_MESSAGE_EOS:
    g_print("End of stream\n");
    exit(1);
    break;

  case GST_MESSAGE_ERROR: {
    gchar *debug;
    GError *error;

    gst_message_parse_error(msg, &error, &debug);
    g_free(debug);

    g_printerr("Error: %s\n", error->message);
    g_error_free(error);
    exit(1);
    break;
  }
  default:
    break;
  }

  return TRUE;
}

GstElement *gstreamer_recieve_create_pipeline(char *pipeline) {
  gst_init(NULL, NULL);
  GError *error = NULL;
  return gst_parse_launch(pipeline, &error);
}

void gstreamer_recieve_start_pipeline(GstElement *pipeline) {
  GstElement *source, *demuxer, *decoder, *conv, *sink;
  GstBus *bus;
  guint bus_watch_id;

  bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  bus_watch_id = gst_bus_add_watch(bus, gstreamer_recieve_bus_call, main_loop);
  gst_object_unref(bus);

  gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void gstreamer_recieve_stop_pipeline(GstElement *pipeline) { gst_element_set_state(pipeline, GST_STATE_NULL); }

void gstreamer_recieve_push_buffer(GstElement *pipeline, void *buffer, int len) {
  GstElement *src = gst_bin_get_by_name(GST_BIN(pipeline), "src");
  if (src != NULL) {
    gpointer p = g_memdup(buffer, len);
    GstBuffer *buffer = gst_buffer_new_wrapped(p, len);
    gst_app_src_push_buffer(GST_APP_SRC(src), buffer);
  }
}
