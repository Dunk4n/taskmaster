/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   conv_pointeur.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/14 11:08:54 by niduches          #+#    #+#             */
/*   Updated: 2019/10/20 18:18:10 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <wchar.h>
#include "printf.h"

static void		display_hex(int output, long ptr, int size)
{
	char	res[13];
	size_t	i;

	if (!ptr)
	{
		write(output, "0", 1);
		return ;
	}
	res[size] = '\0';
	i = 0;
	while (ptr)
	{
		i++;
		res[--size] = HEX[ptr % 16];
		ptr /= 16;
	}
	write(output, res, i);
}

static size_t	display_ptr(int output, long ptr, int min)
{
	int		i;
	int		size;
	long	tmp;

	write(output, "0x", 2);
	i = 0;
	min -= 2;
	tmp = ptr;
	size = 1;
	while (tmp /= 16)
		size++;
	while (i < min - size)
	{
		write(output, "0", 1);
		i++;
	}
	display_hex(output, ptr, size);
	return (i);
}

static size_t	size_ptr(int output, long ptr, int *flags)
{
	size_t	i;

	i = 0;
	if (!ptr && !flags[2])
	{
		write(output, "0x", 2);
		return (2);
	}
	if (!ptr)
		return (3);
	while (ptr /= 16)
		i++;
	return (i + 3);
}

int				conv_ptr(int output, va_list list, int *flags)
{
	void	*ptr;
	size_t	i;

	if (flags[2] >= 0 || flags[0])
		flags[1] = 0;
	ptr = va_arg(list, void*);
	i = size_ptr(output, (long)ptr, flags);
	if (flags[0] && (flags[2] || ptr))
		i += display_ptr(output, (long)ptr, flags[2] + 2);
	while (!flags[1] && (int)i < flags[10])
	{
		write(output, " ", 1);
		i++;
	}
	if (!flags[0] && (flags[2] || ptr))
	{
		if (flags[1])
			i += display_ptr(output, (long)ptr, flags[10]);
		else
			i += display_ptr(output, (long)ptr, flags[2] + 2);
	}
	return (i);
}
