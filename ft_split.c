/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ysabik <ysabik@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/24 12:05:29 by ysabik            #+#    #+#             */
/*   Updated: 2023/07/25 05:10:36 by ysabik           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>

char	**ft_split(char *str, char *charset);
int		ft_split_len(char *str, char *charset);
void	ft_split_fill(char *str, char *charset, char **splitted);
void	ft_count(int *counter, int *str_j, char *str, char *charset);
int		ft_is_inside_of(char c, char *str);

char	**ft_split(char *str, char *charset)
{
	int		len;
	char	**splitted;

	len = ft_split_len(str, charset);
	splitted = malloc(sizeof(char *) * (len + 1));
	ft_split_fill(str, charset, splitted);
	splitted[len] = 0;
	return (splitted);
}

int	ft_split_len(char *str, char *charset)
{
	int	i;
	int	j;
	int	tab_len;
	int	counter;

	tab_len = 0;
	i = -1;
	while (i == -1 || str[i])
	{
		if (i == -1 || ft_is_inside_of(str[i], charset))
		{
			counter = 0;
			j = i + 1;
			while (str[j] && !ft_is_inside_of(str[j], charset))
			{
				counter++;
				j++;
				i++;
			}
			if (counter)
				tab_len += 1;
		}
		i++;
	}
	return (tab_len);
}

void	ft_split_fill(char *str, char *charset, char **splitted)
{
	int	str_i;
	int	str_j;
	int	tab_index;
	int	counter;

	tab_index = 0;
	str_i = -2;
	while (++str_i == -1 || str[str_i])
	{
		if (str_i == -1 || ft_is_inside_of(str[str_i], charset))
		{
			str_j = str_i + 1;
			ft_count(&counter, &str_j, str, charset);
			if (counter)
			{
				splitted[tab_index] = malloc(sizeof(char) * (counter + 1));
				str_j = -1;
				while (++str_j < counter)
					splitted[tab_index][str_j] = str[str_i + 1 + str_j];
				splitted[tab_index][str_j] = '\0';
				tab_index++;
			}
			str_i += counter;
		}
	}
}

void	ft_count(int *counter, int *str_j, char *str, char *charset)
{
	*counter = 0;
	while (str[*str_j] && !ft_is_inside_of(str[*str_j], charset))
	{
		*counter += 1;
		*str_j += 1;
	}
}

int	ft_is_inside_of(char c, char *str)
{
	int	i;

	i = 0;
	while (str[i])
		if (str[i] == c)
			return (1);
	else
		i++;
	return (0);
}
