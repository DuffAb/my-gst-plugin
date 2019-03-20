/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2019 ldf <<user@hostname.org>>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-myfilter
 *
 * FIXME:Describe myfilter here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! myfilter ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <gst/audio/audio.h>
#include "gstmyfilter.h"

GST_DEBUG_CATEGORY_STATIC (gst_my_filter_debug);
#define GST_CAT_DEFAULT gst_my_filter_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 * GstStaticPadTemplate是 elsment 将(或可能)创建和使用的pad的描述。
 */
 #define AMPLIFY_CAPS_STRING    \
  "audio/x-raw, "                     \
  "channels = (int) 1, "              \
  "rate = (int) 44100, "              \
  "layout = (string) interleaved, "   \
  "format = (string) " GST_AUDIO_NE(S16)
 #define MY_FILTER_STRING \
 "audio/x-raw, "    \
 "format = (string) " GST_AUDIO_NE(S16) ", "  \
 "channels = (int) { 1, 2 }, "    \
 "rate = (int) [ 8000, 96000 ]"
 
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink", //pad的简称
    GST_PAD_SINK,           //pad的方向
    GST_PAD_ALWAYS,         //pad存在属性，always pad 、sometimes pad、request pad
    //GST_STATIC_CAPS ("ANY") //这个element(caps)支持的数据类型 "ANY":意味着这个element将接受所有输入
                              //在实际情况中，可以设置媒体类型和一组属性，以确保只输入受支持的内容
    GST_STATIC_CAPS (
      "audio/x-raw, "
      "format = (string) " GST_AUDIO_NE (S16) ", "
      "channels = (int) { 1, 2 }, "
      "rate = (int) [ 8000, 96000 ]")
      //音频过滤器
      //支持原始整数16位音频
      //单声道
      //音频采样率
  );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

#define gst_my_filter_parent_class parent_class
G_DEFINE_TYPE (GstMyFilter, gst_my_filter, GST_TYPE_ELEMENT);

static void gst_my_filter_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_my_filter_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_my_filter_sink_event (GstPad * pad, GstObject * parent, GstEvent * event);
static GstFlowReturn gst_my_filter_chain (GstPad * pad, GstObject * parent, GstBuffer * buf);
static gboolean gst_my_filter_src_query (GstPad  * pad, GstObject * parent, GstQuery * query);
static GstStateChangeReturn gst_my_filter_change_state (GstElement *element, GstStateChange transition);


/* GObject vmethod implementations */

/* initialize the myfilter's class */
/* 只用于初始化类一次(指定类具有哪些信号、参数和虚函数，并设置全局状态) */
static void
gst_my_filter_class_init (GstMyFilterClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_my_filter_set_property;
  gobject_class->get_property = gst_my_filter_get_property;
  gstelement_class->change_state = gst_my_filter_change_state;

  /* 控制 element行为的主要和最重要的方法是通过GObject属性。GObject属性在 xxx_class_init()函数中定义 */
  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));

  gst_element_class_set_details_simple(gstelement_class,
    "MyFilter",                         //元素的长英文名称
    "FIXME:Generic",                    //元素的类型
    "FIXME:Generic Template Element",   //元素用途的简要描述
    "ldf <<user@hostname.org>>");       //元素作者的名称，可选地后跟尖括号中的联系电子邮件地址

  //gst_element_class_add_pad_template()在_class_init()函数中注册这些pad模板
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory)); //需要一个句柄GstPadTemplate，可以使用 gst_static_pad_template_get()从静态pad模板创建这个句柄
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 * 用于初始化这种类型(GstMyFilter)的特定实例
 */
static void
gst_my_filter_init (GstMyFilter * filter)
{
  //1. 使用 gst_pad_new_from_static_template()从静态模板(sink_factory)中创建pad
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");

  /* 设置 xxx_event()函数指针
   * configure event function on the pad before adding the pad to the element*/
  gst_pad_set_event_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_my_filter_sink_event));
  //设置一个 xxx_chain()函数指针，它将接收和处理sinkpad上的输入数据
  gst_pad_set_chain_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_my_filter_chain));
  GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
  //将pad与element注册
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);


  /* pad through which data goes out of the element */
  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  /* pads are configured here with gst_pad_set_*_function () */
  /* [...] */
  GST_PAD_SET_PROXY_CAPS (filter->srcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

  /* configure event function on the pad before adding
   * the pad to the element */
  gst_pad_set_query_function (filter->srcpad,
      gst_my_filter_src_query);

  /* properties initial value */
  filter->silent = FALSE;
}

static void
gst_my_filter_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstMyFilter *filter = GST_MYFILTER (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      g_print ("Silent argument was changed to %s\n",
           filter->silent ? "true" : "false");
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_my_filter_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstMyFilter *filter = GST_MYFILTER (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static GstStateChangeReturn
gst_my_filter_change_state (GstElement *element, GstStateChange transition)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  GstMyFilter *filter = GST_MY_FILTER (element);

  switch (transition) {
	case GST_STATE_CHANGE_NULL_TO_READY:
	  if (!gst_my_filter_allocate_memory (filter))
		return GST_STATE_CHANGE_FAILURE;
	  break;
	default:
	  break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE)
	return ret;

  switch (transition) {
	case GST_STATE_CHANGE_READY_TO_NULL:
	  gst_my_filter_free_memory (filter);
	  break;
	default:
	  break;
  }

  return ret;
}

/* GstElement vmethod implementations */

/* this function handles sink events
 * xxx_event函数通知你数据流中发生的特殊事件(如caps、end-of-stream、newsegment、tags等)。
 * 事件可以在上游和下游传播，因此你可以在src pad和sink pad上接收它们.
 */
static gboolean
gst_my_filter_sink_event (GstPad * pad, GstObject * parent, GstEvent * event)
{
  GstMyFilter *filter;
  gboolean ret;

  filter = GST_MYFILTER (parent);

  GST_LOG_OBJECT (filter, "Received %s event: %" GST_PTR_FORMAT,
      GST_EVENT_TYPE_NAME (event), event);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS://这个事件默认不转发，必须自己处理
    {
      GstCaps * caps;
      /* we should handle the format here */

      /* push the event downstream */
      gst_event_parse_caps (event, &caps);
      /* do something with the caps */

      /* and forward */
      ret = gst_pad_event_default (pad, parent, event);
      break;
    }
    case GST_EVENT_EOS:
      /* end-of-stream, we should close down all stream leftovers here */
      gst_my_filter_stop_processing (filter);
      ret = gst_pad_event_default (pad, parent, event);
      break;
    default:
      /* just call the default handler
       * 根据事件类型，默认处理程序将转发事件或简单地取消引用它
       */
      ret = gst_pad_event_default (pad, parent, event);
      break;
  }
  return ret;
}

/* chain function
 * this function does the actual processing
 * xxx_chain 是进行所有数据处理的函数
 */
static GstFlowReturn
gst_my_filter_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
  GstMyFilter *filter;
  GstBuffer *outbuf;

  filter = GST_MYFILTER (parent);
  outbuf = gst_my_filter_process_data (filter, buf);
  gst_buffer_unref (buf);

  if (!outbuf) {
    /* something went wrong - signal an error */
    GST_ELEMENT_ERROR (GST_ELEMENT (filter), STREAM, FAILED, (NULL), (NULL));
    return GST_FLOW_ERROR;
  }

  if (filter->silent == FALSE){
    g_print ("I'm plugged, therefore I'm in.\n");
    g_print ("Have data of size %" G_GSIZE_FORMAT" bytes!\n", gst_buffer_get_size (buf));
  }

  /* just push out the incoming buffer without touching it */
  return gst_pad_push (filter->srcpad, outbuf);
}

static gboolean
gst_my_filter_src_query (GstPad *pad, GstObject *parent, GstQuery *query)
{
  gboolean ret;
  GstMyFilter *filter = GST_MY_FILTER (parent);

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_POSITION:
      /* we should report the current position */
      //[...]
      break;
    case GST_QUERY_DURATION:
      /* we should report the duration here */
      //[...]
      break;
    case GST_QUERY_CAPS:
      /* we should report the supported caps here */
      //[...]
      break;
    default:
      /* just call the default handler
       * 对于未知查询，最好调用缺省查询处理程序gst_pad_query_default()。
       * 根据查询类型，默认处理程序将转发查询或简单地取消引用查询 */
      ret = gst_pad_query_default (pad, parent, query);
      break;
  }
  return ret;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 * 它在插件加载后立即调用，并且应该返回TRUE或FALSE，这取决于它是否正确初始化了依赖项。
 * 此外，在这个函数中，应该注册插件中支持的任何元素类型
 */
static gboolean
myfilter_init (GstPlugin * myfilter)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template myfilter' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_my_filter_debug, "myfilter",
      0, "Template myfilter");

  return gst_element_register (myfilter, "myfilter", GST_RANK_NONE,
      GST_TYPE_MYFILTER);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstmyfilter"
#endif

/* gstreamer looks for this structure to register myfilters
 *
 * exchange the string 'Template myfilter' with your myfilter description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    myfilter,
    "Template myfilter",
    myfilter_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
