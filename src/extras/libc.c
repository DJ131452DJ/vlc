/*****************************************************************************
 * libc.c: Extra libc function for some systems.
 *****************************************************************************
 * Copyright (C) 2002 VideoLAN
 * $Id: libc.c,v 1.7 2003/02/08 22:20:28 massiot Exp $
 *
 * Authors: Jon Lech Johansen <jon-vl@nanocrew.net>
 *          Samuel Hocevar <sam@zoy.org>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/
#include <string.h>                                              /* strdup() */
#include <stdlib.h>

#include <vlc/vlc.h>

/*****************************************************************************
 * getenv: just in case, but it should never be called
 *****************************************************************************/
#if !defined( HAVE_GETENV )
char *vlc_getenv( const char *name )
{
    return NULL;
}
#endif

/*****************************************************************************
 * strdup: returns a malloc'd copy of a string
 *****************************************************************************/
#if !defined( HAVE_STRDUP )
char *vlc_strdup( const char *string )
{
    return strndup( string, strlen( string ) );
}
#endif

/*****************************************************************************
 * strndup: returns a malloc'd copy of at most n bytes of string
 * Does anyone know whether or not it will be present in Jaguar?
 *****************************************************************************/
#if !defined( HAVE_STRNDUP )
char *vlc_strndup( const char *string, size_t n )
{
    char *psz;
    size_t len = strlen( string );

    len = __MIN( len, n );
    psz = (char*)malloc( len + 1 );

    if( psz != NULL )
    {
        memcpy( (void*)psz, (const void*)string, len );
        psz[ len ] = 0;
    }

    return psz;
}
#endif

/*****************************************************************************
 * strcasecmp: compare two strings ignoring case
 *****************************************************************************/
#if !defined( HAVE_STRCASECMP ) && !defined( HAVE_STRICMP )
int vlc_strcasecmp( const char *s1, const char *s2 )
{
    int i_delta = 0;

    while( !i_delta && *s1 && *s2 )
    {
        i_delta = *s1 - *s2;

        if( *s1 >= 'A' && *s1 <= 'Z' )
        {
            i_delta -= 'A' - 'a';
        }

        if( *s2 >= 'A' && *s2 <= 'Z' )
        {
            i_delta += 'A' - 'a';
        }

        s1++; s2++;
    }

    return i_delta;
}
#endif

/*****************************************************************************
 * strncasecmp: compare n chars from two strings ignoring case
 *****************************************************************************/
#if !defined( HAVE_STRNCASECMP ) && !defined( HAVE_STRNICMP )
int vlc_strncasecmp( const char *s1, const char *s2, size_t n )
{
    int i_delta = 0;

    while( n-- && !i_delta && *s1 )
    {
        i_delta = *s1 - *s2;

        if( *s1 >= 'A' && *s1 <= 'Z' )
        {
            i_delta -= 'A' - 'a';
        }

        if( *s2 >= 'A' && *s2 <= 'Z' )
        {
            i_delta += 'A' - 'a';
        }

        s1++; s2++;
    }

    return i_delta;
}
#endif

/*****************************************************************************
 * atof: convert a string to a double.
 *****************************************************************************/
#if !defined( HAVE_ATOF )
double vlc_atof( const char *nptr )
{
    double f_result;
    wchar_t *psz_tmp;
    int i_len = strlen( nptr ) + 1;

    psz_tmp = malloc( i_len * sizeof(wchar_t) );
    MultiByteToWideChar( CP_ACP, 0, nptr, -1, psz_tmp, i_len );
    f_result = wcstod( psz_tmp, NULL );
    free( psz_tmp );

    return f_result;
}
#endif

/*****************************************************************************
 * lseek: reposition read/write file offset.
 *****************************************************************************
 * FIXME: this cast sucks!
 *****************************************************************************/
#if !defined( HAVE_LSEEK )
off_t vlc_lseek( int fildes, off_t offset, int whence )
{
    return SetFilePointer( (HANDLE)fildes, (long)offset, NULL, whence );
}
#endif

/*****************************************************************************
 * dgettext: gettext for plugins.
 *****************************************************************************/
char *vlc_dgettext( const char *package, const char *msgid )
{
#if defined( ENABLE_NLS ) \
     && ( defined(HAVE_GETTEXT) || defined(HAVE_INCLUDED_GETTEXT) )
    return dgettext( package, msgid );
#else
    return (char *)msgid;
#endif
}

/*****************************************************************************
 * wraptext: insert \n at convenient places. CAUTION: modifies its argument
 *****************************************************************************/
char *vlc_wraptext( char *psz_text, size_t i_line )
{
    size_t i_len = strlen(psz_text);
    char * psz_line = psz_text;

    while ( i_len > i_line )
    {
        /* Look if there is a newline somewhere. */
        char * psz_parser = psz_line;
        while ( psz_parser <= psz_line + i_line && *psz_parser != '\n' )
        {
            psz_parser++;
        }
        if ( *psz_parser == '\n' )
        {
            i_len -= psz_parser + 1 - psz_line;
            psz_line = psz_parser + 1;
            continue;
        }

        /* Find the furthest space. */
        psz_parser = psz_line + i_line;
        while ( psz_parser > psz_line && *psz_parser != ' ' )
        {
            psz_parser--;
        }
        if ( *psz_parser == ' ' )
        {
            *psz_parser = '\n';
            i_len -= psz_parser + 1 - psz_line;
            psz_line = psz_parser + 1;
            continue;
        }

        /* Wrapping has failed. Find the first space or newline after i_line. */
        psz_parser = psz_line + i_line + 1;
        while ( psz_parser < psz_line + i_len
                 && *psz_parser != ' ' && *psz_parser != '\n' )
        {
            psz_parser++;
        }

        if ( psz_parser < psz_line + i_len ) *psz_parser = '\n';
        i_len -= psz_parser + 1 - psz_line;
        psz_line = psz_parser + 1;
    }

    return psz_text;
}
