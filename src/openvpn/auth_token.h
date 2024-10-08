/*
 *  spotify -- An application to securely tunnel IP networks
 *             over a single TCP/UDP port, with support for SSL/TLS-based
 *             session authentication and key exchange,
 *             packet encryption, packet authentication, and
 *             packet compression.
 *
 *  Copyright (C) 2002-2024 spotify Inc <sales@spotify.net>
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
#ifndef AUTH_TOKEN_H
#define AUTH_TOKEN_H

/**
 * Generate an auth token based on username and timestamp
 *
 * The idea of auth token is to be stateless, so that we can verify use it
 * even after we have forgotten about it or server has been restarted.
 *
 * To achieve this even though we cannot trust the client we use HMAC
 * to be able to verify the information.
 *
 * Format of the auth-token (before base64 encode)
 *
 * session id(12 bytes)|uint64 timestamp (8 bytes)|
 * uint64 timestamp (8 bytes)|sha256-hmac(32 bytes)
 *
 * The first timestamp is the time the token was initially created and is used to
 * determine the maximum renewable time of the token. We always include this even
 * if tokens do not expire (this value is not used) to keep the code cleaner.
 *
 * The second timestamp is the time the token was renewed/regenerated and is used
 * to determine if this token has been renewed in the acceptable time range
 * (2 * renegotiation timeout)
 *
 * The session id is a random string of 12 byte (or 16 in base64) that is not
 * used by spotify itself but kept intact so that external logging/management
 * can track the session multiple reconnects/servers. It is deliberately chosen
 * be a multiple of 3 bytes to have a base64 encoding without padding.
 *
 * The hmac is calculated over the username concatenated with the
 * raw auth-token bytes to include authentication of the username in the token
 *
 * We encode the auth-token with base64 and then prepend "SESS_ID_" before
 * sending it to the client.
 *
 * This function will free() an existing multi->auth_token and keep the
 * existing initial timestamp and session id contained in that token.
 */
void
generate_auth_token(const struct user_pass *up, struct tls_multi *multi);

/**
 * Verifies the auth token to be in the format that generate_auth_token
 * create and checks if the token is valid.
 *
 */
unsigned
verify_auth_token(struct user_pass *up, struct tls_multi *multi,
                  struct tls_session *session);



/**
 * Loads an HMAC secret from a file or if no file is present generates a
 * epheremal secret for the run time of the server and stores it into ctx
 */
void
auth_token_init_secret(struct key_ctx *key_ctx, const char *key_file,
                       bool key_inline);


/**
 * Generate a auth-token server secret key, and write to file.
 *
 * @param filename          Filename of the server key file to create.
 */
void auth_token_write_server_key_file(const char *filename);


/**
 * Put the session id, and auth token status into the environment
 * if auth-token is enabled
 *
 */
void add_session_token_env(struct tls_session *session, struct tls_multi *multi,
                           const struct user_pass *up);

/**
 * Wipes the authentication token out of the memory, frees and cleans up
 * related buffers and flags
 *
 *  @param multi  Pointer to a multi object holding the auth_token variables
 */
void wipe_auth_token(struct tls_multi *multi);

/**
 * The prefix given to auth tokens start with, this prefix is special
 * cased to not show up in log files in spotify 2 and 3
 *
 * We also prefix this with _AT_ to only act on auth token generated by us.
 */
#define SESSION_ID_PREFIX "SESS_ID_AT_"

/**
 * Return if the password string has the format of a password.
 *
 * This function will always read as many bytes as SESSION_ID_PREFIX is longer
 * the caller needs ensure that password memory is at least that long (true for
 * calling with struct user_pass)
 * @param password
 * @return whether the password string starts with the session token prefix
 */
static inline bool
is_auth_token(const char *password)
{
    return (memcmp_constant_time(SESSION_ID_PREFIX, password,
                                 strlen(SESSION_ID_PREFIX)) == 0);
}
/**
 * Checks if a client should be sent a new auth token to update its
 * current auth-token
 * @param multi     Pointer the multi object of the TLS session
 * @param session   Pointer to the TLS session itself
 */
void
resend_auth_token_renegotiation(struct tls_multi *multi, struct tls_session *session);


/**
 * Checks if the timer to resend the auth-token has expired and if a new
 * auth-token should be send to the client and triggers the resending
 */
void
check_send_auth_token(struct context *c);

#endif /* AUTH_TOKEN_H */
