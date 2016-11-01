/* gr-search-page.c:
 *
 * Copyright (C) 2016 Matthias Clasen <mclasen@redhat.com>
 *
 * Licensed under the GNU General Public License Version 3
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more edit.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gr-search-page.h"
#include "gr-recipe-store.h"
#include "gr-recipe.h"
#include "gr-recipe-tile.h"
#include "gr-app.h"
#include "gr-utils.h"


struct _GrSearchPage
{
        GtkBox     parent_instance;

        GtkWidget *flow_box;
        char *term;
};

G_DEFINE_TYPE (GrSearchPage, gr_search_page, GTK_TYPE_BOX)

static void
search_page_finalize (GObject *object)
{
        GrSearchPage *self = GR_SEARCH_PAGE (object);

        g_clear_pointer (&self->term, g_free);

        G_OBJECT_CLASS (gr_search_page_parent_class)->finalize (object);
}

static void
gr_search_page_init (GrSearchPage *page)
{
        gtk_widget_set_has_window (GTK_WIDGET (page), FALSE);
        gtk_widget_init_template (GTK_WIDGET (page));
}

static void
gr_search_page_class_init (GrSearchPageClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

        object_class->finalize = search_page_finalize;

        gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Recipes/gr-search-page.ui");

        gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), GrSearchPage, flow_box);
}

GtkWidget *
gr_search_page_new (void)
{
        GrSearchPage *page;

        page = g_object_new (GR_TYPE_SEARCH_PAGE, NULL);

        return GTK_WIDGET (page);
}

static void
check_match (GtkWidget *child, gpointer data)
{
        GtkWidget *tile;
        GrRecipe *recipe;
        const char *term = data;

        tile = gtk_bin_get_child (GTK_BIN (child));
        recipe = gr_recipe_tile_get_recipe (GR_RECIPE_TILE (tile));

        if (!gr_recipe_matches (recipe, term))
                gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (child)), child);
}

void
gr_search_page_update_search (GrSearchPage *page, const char *term)
{
        GrRecipeStore *store;
        g_autofree char **keys = NULL;
        g_autofree char *cf_term = NULL;
        guint length;
        int i;

        if (strlen (term) < 3)
                return;

        cf_term = g_utf8_casefold (term, -1);

        if (page->term && g_str_has_prefix (term, page->term)) {
                /* narrowing search */
                gtk_container_forall (GTK_CONTAINER (page->flow_box), check_match, (gpointer)term);
        }
        else {
                container_remove_all (GTK_CONTAINER (page->flow_box));

                store = gr_app_get_recipe_store (GR_APP (g_application_get_default ()));
                keys = gr_recipe_store_get_recipe_keys (store, &length);

                for (i = 0; i < length; i++) {
                        g_autoptr(GrRecipe) recipe = NULL;
                        GtkWidget *tile;

                        recipe = gr_recipe_store_get (store, keys[i]);
                        if (gr_recipe_matches (recipe, term)) {
                                tile = gr_recipe_tile_new (recipe);
                                gtk_widget_show (tile);
                                gtk_container_add (GTK_CONTAINER (page->flow_box), tile);
                        }
                }
        }

        g_free (page->term);
        page->term = g_strdup (term);
}