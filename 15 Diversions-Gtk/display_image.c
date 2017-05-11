#include <gtk/gtk.h>

int main( int argc, char *argv[])
{
    GtkWidget *window, *image;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    image = gtk_image_new_from_file("jan-small.png");
    gtk_container_add(GTK_CONTAINER(window), image);

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show(image);
    gtk_widget_show(window);
    gtk_main();

    return 0;
}
