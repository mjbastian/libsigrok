/*
 * This file is part of the sigrok project.
 *
 * Copyright (C) 2010 Uwe Hermann <uwe@hermann-uwe.de>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <sigrok.h>
#include "config.h"

struct context {
	unsigned int num_enabled_probes;
	unsigned int unitsize;
	char *probelist[65];
	char *header;
};

const char *gnuplot_header = "\
# Sample data in space-separated columns format usable by gnuplot\n\
#\n\
# Generated by: %s on %s%s\
# Timescale: %d %s\n\
#\n\
# Column\tProbe\n\
# -------------------------------------\
----------------------------------------\n\
# 0\t\tSample counter (for internal gnuplot purposes)\n%s\n";

const char *gnuplot_header_comment = "\
# Comment: Acquisition with %d/%d probes at %s\n";

static int init(struct output *o)
{
/* Maximum header length */
#define MAX_HEADER_LEN 2048

	struct context *ctx;
	struct probe *probe;
	GSList *l;
	uint64_t samplerate;
	unsigned int i;
	int b, num_probes;
	char *c, *samplerate_s;
	char wbuf[1000], comment[128];
	time_t t;

	if (!(ctx = calloc(1, sizeof(struct context))))
		return SIGROK_ERR_MALLOC;

	o->internal = ctx;
	ctx->num_enabled_probes = 0;
	for (l = o->device->probes; l; l = l->next) {
		probe = l->data;
		if (probe->enabled)
			ctx->probelist[ctx->num_enabled_probes++] = probe->name;
	}

	ctx->probelist[ctx->num_enabled_probes] = 0;
	ctx->unitsize = (ctx->num_enabled_probes + 7) / 8;

	/* TODO: Allow for configuration via o->param. */

	if (!(ctx->header = calloc(1, MAX_HEADER_LEN + 1))) {
		free(ctx);
		return SIGROK_ERR_MALLOC;
	}

	num_probes = g_slist_length(o->device->probes);
	/* TODO: Handle num_probes == 0, too many probes, etc. */

	comment[0] = '\0';
	if (o->device->plugin) {
		samplerate = *((uint64_t *) o->device->plugin->get_device_info(
				o->device->plugin_index, DI_CUR_SAMPLERATE));
		if (!(samplerate_s = sigrok_samplerate_string(samplerate))) {
			free(ctx->header);
			free(ctx);
			return SIGROK_ERR;
		}
		snprintf(comment, 127, gnuplot_header_comment,
			 ctx->num_enabled_probes, num_probes, samplerate_s);
		free(samplerate_s);
	}

	/* Columns / channels */
	wbuf[0] = '\0';
	for (i = 0; i < ctx->num_enabled_probes; i++) {
		c = (char *)&wbuf + strlen((char *)&wbuf);
		sprintf(c, "# %d\t\t%s\n", i + 1, ctx->probelist[i]);
	}

	/* TODO: date: File or signals? Make y/n configurable. */
	/* TODO: Timescale */
	t = time(NULL);
	b = snprintf(ctx->header, MAX_HEADER_LEN, gnuplot_header,
		     PACKAGE_STRING, ctime(&t), comment, 1, "ns",
		     (char *)&wbuf);

	/* TODO: Handle snprintf errors. */

	return 0;
}

static int event(struct output *o, int event_type, char **data_out,
		 uint64_t *length_out)
{
	struct context *ctx;

	ctx = o->internal;
	switch (event_type) {
	case DF_TRIGGER:
		/* TODO */
		break;
	case DF_END:
		*data_out = NULL;
		*length_out = 0;
		free(o->internal);
		o->internal = NULL;
		break;
	}

	return SIGROK_OK;
}

static int data(struct output *o, char *data_in, uint64_t length_in,
		char **data_out, uint64_t *length_out)
{
	struct context *ctx;
	unsigned int i, outsize, p, curbit;
	uint64_t sample;
	static uint64_t samplecount = 0;
	char *outbuf, *c;

	ctx = o->internal;
	outsize = 0;
	if (ctx->header)
		outsize = strlen(ctx->header);

	/* FIXME: Use realloc(). */
	if (!(outbuf = calloc(1, outsize + 1 + 1000000)))
		return SIGROK_ERR_MALLOC; /* TODO: free()? What to free? */

	outbuf[0] = '\0';
	if (ctx->header) {
		/* The header is still here, this must be the first packet. */
		strncpy(outbuf, ctx->header, outsize);
		free(ctx->header);
		ctx->header = NULL;
	}

	/* TODO: Are disabled probes handled correctly? */

	for (i = 0; i <= length_in - ctx->unitsize; i += ctx->unitsize) {
		memcpy(&sample, data_in + i, ctx->unitsize);

		/* The first column is a counter (needed for gnuplot). */
		c = outbuf + strlen(outbuf);
		sprintf(c, "%" PRIu64 "\t\t", samplecount++);

		/* The next columns are the values of all channels. */
		for (p = 0; p < ctx->num_enabled_probes; p++) {
			curbit = (sample & ((uint64_t) (1 << p))) >> p;
			c = outbuf + strlen(outbuf);
			sprintf(c, "%d ", curbit);
		}

		c = outbuf + strlen(outbuf);
		sprintf(c, "\n");

		/* TODO: realloc() if strlen(outbuf) is almost "full"... */
	}

	*data_out = outbuf;
	*length_out = strlen(outbuf);

	return SIGROK_OK;
}

struct output_format output_gnuplot = {
	"gnuplot",
	"Gnuplot",
	init,
	data,
	event,
};
