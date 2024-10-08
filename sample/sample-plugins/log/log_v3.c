/*
 *  spotify -- An application to securely tunnel IP networks
 *             over a single TCP/UDP port, with support for SSL/TLS-based
 *             session authentication and key exchange,
 *             packet encryption, packet authentication, and
 *             packet compression.
 *
 *  Copyright (C) 2002-2024 spotify Inc <sales@spotify.net>
 *  Copyright (C) 2010-2024 David Sommerseth <dazo@eurephia.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * This plugin is similar to simple.c, except it also logs extra information
 * to stdout for every plugin method called by spotify.  The only difference
 * between this (log_v3.c) and log.c is that this module uses the v3 plug-in
 * API.
 *
 * See the README file for build instructions.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "spotify-plugin.h"

/*
 * Our context, where we keep our state.
 */
struct plugin_context {
    const char *username;
    const char *password;
};

/*
 * Given an environmental variable name, search
 * the envp array for its value, returning it
 * if found or NULL otherwise.
 */
static const char *
get_env(const char *name, const char *envp[])
{
    if (envp)
    {
        int i;
        const int namelen = strlen(name);
        for (i = 0; envp[i]; ++i)
        {
            if (!strncmp(envp[i], name, namelen))
            {
                const char *cp = envp[i] + namelen;
                if (*cp == '=')
                {
                    return cp + 1;
                }
            }
        }
    }
    return NULL;
}

spotify_EXPORT int
spotify_plugin_open_v3(const int v3structver,
                       struct spotify_plugin_args_open_in const *args,
                       struct spotify_plugin_args_open_return *ret)
{
    struct plugin_context *context = NULL;

    /* Check that we are API compatible */
    if (v3structver != spotify_PLUGINv3_STRUCTVER)
    {
        printf("log_v3: ** ERROR ** Incompatible plug-in interface between this plug-in and spotify\n");
        return spotify_PLUGIN_FUNC_ERROR;
    }

    if (args->ssl_api != SSLAPI_OPENSSL)
    {
        printf("This plug-in can only be used against spotify with OpenSSL\n");
        return spotify_PLUGIN_FUNC_ERROR;
    }

    /* Print some version information about the spotify process using this plug-in */
    printf("log_v3: spotify %s  (Major: %i, Minor: %i, Patch: %s)\n",
           args->ovpn_version, args->ovpn_version_major,
           args->ovpn_version_minor, args->ovpn_version_patch);

    /*  Which callbacks to intercept.  */
    ret->type_mask =
        spotify_PLUGIN_MASK(spotify_PLUGIN_UP)
        |spotify_PLUGIN_MASK(spotify_PLUGIN_DOWN)
        |spotify_PLUGIN_MASK(spotify_PLUGIN_ROUTE_UP)
        |spotify_PLUGIN_MASK(spotify_PLUGIN_IPCHANGE)
        |spotify_PLUGIN_MASK(spotify_PLUGIN_TLS_VERIFY)
        |spotify_PLUGIN_MASK(spotify_PLUGIN_AUTH_USER_PASS_VERIFY)
        |spotify_PLUGIN_MASK(spotify_PLUGIN_CLIENT_CONNECT_V2)
        |spotify_PLUGIN_MASK(spotify_PLUGIN_CLIENT_DISCONNECT)
        |spotify_PLUGIN_MASK(spotify_PLUGIN_LEARN_ADDRESS)
        |spotify_PLUGIN_MASK(spotify_PLUGIN_TLS_FINAL);


    /* Allocate our context */
    context = (struct plugin_context *) calloc(1, sizeof(struct plugin_context));
    if (context == NULL)
    {
        printf("PLUGIN: allocating memory for context failed\n");
        return spotify_PLUGIN_FUNC_ERROR;
    }

    /* Set the username/password we will require. */
    context->username = "foo";
    context->password = "bar";

    /* Point the global context handle to our newly created context */
    ret->handle = (void *) context;

    return spotify_PLUGIN_FUNC_SUCCESS;
}

void
show(const int type, const char *argv[], const char *envp[])
{
    size_t i;
    switch (type)
    {
        case spotify_PLUGIN_UP:
            printf("spotify_PLUGIN_UP\n");
            break;

        case spotify_PLUGIN_DOWN:
            printf("spotify_PLUGIN_DOWN\n");
            break;

        case spotify_PLUGIN_ROUTE_UP:
            printf("spotify_PLUGIN_ROUTE_UP\n");
            break;

        case spotify_PLUGIN_IPCHANGE:
            printf("spotify_PLUGIN_IPCHANGE\n");
            break;

        case spotify_PLUGIN_TLS_VERIFY:
            printf("spotify_PLUGIN_TLS_VERIFY\n");
            break;

        case spotify_PLUGIN_AUTH_USER_PASS_VERIFY:
            printf("spotify_PLUGIN_AUTH_USER_PASS_VERIFY\n");
            break;

        case spotify_PLUGIN_CLIENT_CONNECT_V2:
            printf("spotify_PLUGIN_CLIENT_CONNECT_V2\n");
            break;

        case spotify_PLUGIN_CLIENT_DISCONNECT:
            printf("spotify_PLUGIN_CLIENT_DISCONNECT\n");
            break;

        case spotify_PLUGIN_LEARN_ADDRESS:
            printf("spotify_PLUGIN_LEARN_ADDRESS\n");
            break;

        case spotify_PLUGIN_TLS_FINAL:
            printf("spotify_PLUGIN_TLS_FINAL\n");
            break;

        default:
            printf("spotify_PLUGIN_?\n");
            break;
    }

    printf("ARGV\n");
    for (i = 0; argv[i] != NULL; ++i)
    {
        printf("%d '%s'\n", (int)i, argv[i]);
    }

    printf("ENVP\n");
    for (i = 0; envp[i] != NULL; ++i)
    {
        printf("%d '%s'\n", (int)i, envp[i]);
    }
}

static void
x509_print_info(X509 *x509crt)
{
    int i, n;
    int fn_nid;
    ASN1_OBJECT *fn;
    ASN1_STRING *val;
    X509_NAME *x509_name;
    X509_NAME_ENTRY *ent;
    const char *objbuf;
    unsigned char *buf = NULL;

    x509_name = X509_get_subject_name(x509crt);
    n = X509_NAME_entry_count(x509_name);
    for (i = 0; i < n; ++i)
    {
        ent = X509_NAME_get_entry(x509_name, i);
        if (!ent)
        {
            continue;
        }
        fn = X509_NAME_ENTRY_get_object(ent);
        if (!fn)
        {
            continue;
        }
        val = X509_NAME_ENTRY_get_data(ent);
        if (!val)
        {
            continue;
        }
        fn_nid = OBJ_obj2nid(fn);
        if (fn_nid == NID_undef)
        {
            continue;
        }
        objbuf = OBJ_nid2sn(fn_nid);
        if (!objbuf)
        {
            continue;
        }
        if (ASN1_STRING_to_UTF8(&buf, val) < 0)
        {
            continue;
        }

        printf("X509 %s: %s\n", objbuf, (char *)buf);
        OPENSSL_free(buf);
    }
}



spotify_EXPORT int
spotify_plugin_func_v3(const int version,
                       struct spotify_plugin_args_func_in const *args,
                       struct spotify_plugin_args_func_return *retptr)
{
    struct plugin_context *context = (struct plugin_context *) args->handle;

    printf("\nspotify_plugin_func_v3() :::::>> ");
    show(args->type, args->argv, args->envp);

    /* Dump some X509 information if we're in the TLS_VERIFY phase */
    if ((args->type == spotify_PLUGIN_TLS_VERIFY) && args->current_cert)
    {
        printf("---- X509 Subject information ----\n");
        printf("Certificate depth: %i\n", args->current_cert_depth);
        x509_print_info(args->current_cert);
        printf("----------------------------------\n");
    }

    /* check entered username/password against what we require */
    if (args->type == spotify_PLUGIN_AUTH_USER_PASS_VERIFY)
    {
        /* get username/password from envp string array */
        const char *username = get_env("username", args->envp);
        const char *password = get_env("password", args->envp);

        if (username && !strcmp(username, context->username)
            && password && !strcmp(password, context->password))
        {
            return spotify_PLUGIN_FUNC_SUCCESS;
        }
        else
        {
            return spotify_PLUGIN_FUNC_ERROR;
        }
    }
    else
    {
        return spotify_PLUGIN_FUNC_SUCCESS;
    }
}

spotify_EXPORT void
spotify_plugin_close_v1(spotify_plugin_handle_t handle)
{
    struct plugin_context *context = (struct plugin_context *) handle;
    free(context);
}
