/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   printf.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: niduches <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/19 16:12:52 by niduches          #+#    #+#             */
/*   Updated: 2020/04/23 10:33:42 by niduches         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PRINTF_H
# define PRINTF_H

# include <stdarg.h>

# define HEX "0123456789abcdef"
# define UHEX "0123456789ABCDEF"
# define CONVERSIONS "cspdiuxX%nfge"
# define FLAGS "-0.*lh\'# +"
# define NB_FLAGS 10

int				ft_vfprintf(int output, const char *str, va_list list);
int				ft_fprintf(int output, const char *str, ...);
int				ft_printf(const char *str, ...);
size_t			ft_putnstr(int output, const char *str, size_t n);
int				ft_isflag(char c);
int				ft_isconv(char c);
int				ft_makeconv(int output, const char *str, int *size,
va_list list, size_t nb_print);
void			get_flags(int *flags, const char *str, size_t size,
va_list list);
int				conv_char(int output, va_list list, int *flags);
int				conv_str(int output, va_list list, int *flags);
int				conv_ptr(int output, va_list list, int *flags);
int				conv_int(int output, va_list list, int *flags);
int				conv_uint(int output, va_list list, int *flags);
int				conv_hex(int output, va_list list, int *flags);
int				conv_uhex(int output, va_list list, int *flags);
int				conv_nb_print_char(int output, va_list list, int *flags);
int				conv_float_print(int output, double nb, int *flags, int round);
int				conv_sct_print(int output, double nb, int *flags, int round);
int				conv_g(int output, va_list list, int *flags);
int				make_g(int output, double nb, int *flags);
size_t			put_long_nbr(int output, long long nb, int len, int nb_c);
double			float_round(double nb, int *flags, int round);

#endif
