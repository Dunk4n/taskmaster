/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conv_str.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/13 20:09:48 by niduches          #+#    #+#             */
/*   Updated: 2019/10/21 14:45:36 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <wchar.h>
#include "printf.h"

static char		*get_str(va_list list, int *flags)
{
	if (flags[4] == 1)
		return ((char*)va_arg(list, wchar_t*));
	if (flags[4] == 2)
		return (va_arg(list, char*));
	if (flags[5] == 1)
		return (va_arg(list, char*));
	if (flags[5] == 2)
		return (va_arg(list, char*));
	return (va_arg(list, char*));
}

static int		get_len(int *flags, size_t len)
{
	if (flags[2] < 0)
		return ((int)len);
	return ((len < (size_t)flags[2]) ? (int)len : flags[2]);
}

int				conv_str(int output, va_list list, int *flags)
{
	char	*s;
	size_t	i;
	size_t	len;

	s = get_str(list, flags);
	if (!s)
		s = "(null)";
	len = 0;
	while (s[len])
		len++;
	flags[2] = get_len(flags, len);
	if (flags[0])
		write(output, s, flags[2]);
	i = 0;
	while (flags[10] - flags[2] > 0 && (int)i < (int)(flags[10] - flags[2]))
	{
		write(output, (flags[1] && !flags[0]) ? "0" : " ", 1);
		i++;
	}
	if (!flags[0])
		write(output, s, ((int)len < flags[2]) ? (int)len : flags[2]);
	return (i + (((int)len < flags[2]) ? (int)len : flags[2]));
}
