#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "date.h"

#define THURSDAY 4 /* for reformation */
#define SATURDAY 6 /* 1 Jan 1 was a Saturday */

#define FIRST_MISSING_DAY 639787 /* 3 Sep 1752 */
#define NUMBER_MISSING_DAYS 11   /* 11 day correction */

#define MAXDAYS 42 /* max slots in a month array */
#define SPACE -1   /* used in day array */

static int days_in_month[2][13] = {
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
};

int sep1752[MAXDAYS] = {
    SPACE,
    SPACE,
    1,
    2,
    14,
    15,
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    24,
    25,
    26,
    27,
    28,
    29,
    30,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
    SPACE,
},
    j_sep1752[MAXDAYS] = {
        SPACE,
        SPACE,
        245,
        246,
        258,
        259,
        260,
        261,
        262,
        263,
        264,
        265,
        266,
        267,
        268,
        269,
        270,
        271,
        272,
        273,
        274,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
        SPACE,
};

int empty[MAXDAYS];

char *month_names[12] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
};

char *day_headings = " S  M Tu  W Th  F  S";
char *j_day_headings = "  S   M  Tu   W  Th   F   S";

/* leap year -- account for gregorian reformation in 1752 */
#define leap_year(yr) ((yr) <= 1752 ? (!((yr) % 4)) : (!((yr) % 4) && ((yr) % 100)) || (!((yr) % 400)))

/* number of centuries since 1700, not inclusive */
#define centuries_since_1700(yr) ((yr) > 1700 ? (yr) / 100 - 17 : 0)

/* number of centuries since 1700 whose modulo of 400 is 0 */
#define quad_centuries_since_1700(yr) ((yr) > 1600 ? ((yr)-1600) / 400 : 0)

/* number of leap years between year 1 and this year, not inclusive */
#define leap_years_since_year_1(yr) ((yr) / 4 - centuries_since_1700(yr) + quad_centuries_since_1700(yr))

int julian;
#define DAY_LEN 3     /* 3 spaces per day */
#define J_DAY_LEN 4   /* 4 spaces per day */
#define WEEK_LEN 20   /* 7 * 3 - one space at the end */
#define J_WEEK_LEN 27 /* 7 * 4 - one space at the end */
#define HEAD_SEP 2    /* spaces between day headings */
#define J_HEAD_SEP 2

void ascii_day(char *p, int day)
{
  int display, val;
  static char *aday[] = {
      "  ",
      " 1",
      " 2",
      " 3",
      " 4",
      " 5",
      " 6",
      " 7",
      " 8",
      " 9",
      "10",
      "11",
      "12",
      "13",
      "14",
      "15",
      "16",
      "17",
      "18",
      "19",
      "20",
      "21",
      "22",
      "23",
      "24",
      "25",
      "26",
      "27",
      "28",
      "29",
      "30",
      "31",
  };

  if (day == SPACE)
  {
    if (julian)
    {
      memset(p, ' ', J_DAY_LEN);
    }
    else
    {
      memset(p, ' ', DAY_LEN);
    }

    return;
  }
  if (julian)
  {
    if ((val = (day / 100)))
    {
      day %= 100;
      *p++ = val + '0';
      display = 1;
    }
    else
    {
      *p++ = ' ';
      display = 0;
    }
    val = day / 10;
    if (val || display)
      *p++ = val + '0';
    else
      *p++ = ' ';
    *p++ = day % 10 + '0';
  }
  else
  {
    *p++ = aday[day][0];
    *p++ = aday[day][1];
  }
  *p = ' ';
}

void trim_trailing_spaces(char *s)
{
  char *p;

  for (p = s; *p; ++p);

  while (p > s && *--p == ' ');

  if (p > s) p++;
  *p = '\0';
}

void center(char *str, int len, int separate)
{
  len -= strlen(str);

  char spacediv[10];
  strcpy(spacediv, " ");
  for (int i = 1; i < len / 2; i++)
  {
    strcat(spacediv, " ");
  }

  char spacemod[10];
  strcpy(spacemod, " ");
  for (int i = 1; i < (len / 2 + len % 2); i++)
  {
    strcat(spacemod, " ");
  }

  printf(1, "%s%s%s", spacediv, str, spacemod);

  if (separate)
  {
    for (int i = 0; i < separate; i++)
    {
      printf(1, " ");
    }
  }
}

/*
 * day_in_year --
 *	return the 1 based day number within the year
 */
int day_in_year(int day, int month, int year)
{
  int i, leap;

  leap = leap_year(year);
  for (i = 1; i < month; i++)
  {
    day += days_in_month[leap][i];
  }
  return (day);
}

/*
 * day_in_week
 *	return the 0 based day number for any date from 1 Jan. 1 to
 *	31 Dec. 9999.  Assumes the Gregorian reformation eliminates
 *	3 Sep. 1752 through 13 Sep. 1752.  Returns Thursday for all
 *	missing days.
 */
int day_in_week(int day, int month, int year)
{
  long temp;

  temp = (long)(year - 1) * 365 + leap_years_since_year_1(year - 1) + day_in_year(day, month, year);

  if (temp < FIRST_MISSING_DAY)
  {
    return ((temp - 1 + SATURDAY) % 7);
  }
  if (temp >= (FIRST_MISSING_DAY + NUMBER_MISSING_DAYS))
  {
    return (((temp - 1 + SATURDAY) - NUMBER_MISSING_DAYS) % 7);
  }

  return (THURSDAY);
}

/*
 * day_array --
 *	Fill in an array of 42 integers with a calendar.  Assume for a moment
 *	that you took the (maximum) 6 rows in a calendar and stretched them
 *	out end to end.  You would have 42 numbers or spaces.  This routine
 *	builds that array for any month from Jan. 1 through Dec. 9999.
 */
void day_array(int month, int year, int *days)
{
  int day, dw, dm;

  if (month == 9 && year == 1752)
  {
    // bcopy(julian ? j_sep1752 : sep1752, days, MAXDAYS * sizeof(int));

    for (int i = 0; i < MAXDAYS; i++)
    {
      if (julian)
        j_sep1752[i] = days[i];
      else
        sep1752[i] = days[i];
    }
    return;
  }

  for (int i = 0; i < MAXDAYS; i++)
  {
    // printf(1, "day year = %d\n", days[i]);
    empty[i] = days[i];
  }

  dm = days_in_month[leap_year(year)][month];
  dw = day_in_week(1, month, year);

  if (julian)
    day = day_in_year(1, month, year);
  else
    day = 1;

  while (dm--)
    days[dw++] = day++;
}

void monthly(int month, int year)
{
  int col, row;
  char *p;
  int len, days[MAXDAYS];
  char lineout[1024];

  day_array(month, year, days);

  char num[100];
  int count = 0;
  int use = year;
  while (use != 0)
  {
    use /= 10; // angka = angka/10
    count++;
  }
  intToStr(year, num, count);

  strcpy(lineout, month_names[month - 1]);
  strcat(lineout, " ");
  strcat(lineout, num);

  len = strlen(lineout);
  if (julian)
  {
    // printf(1, "julian\n");
    char lenweek[100];

    strcpy(lenweek, " ");
    for (int i = 1; i < (J_WEEK_LEN - len) / 2; i++)
    {
      strcat(lenweek, " ");
    }

    printf(1, "%s%s\n%s\n", lenweek, lineout, j_day_headings);
  }
  else
  {
    char lenweek[100];

    strcpy(lenweek, " ");
    for (int i = 1; i < (WEEK_LEN - len) / 2; i++)
    {
      strcat(lenweek, " ");
    }

    printf(1, "%s%s\n%s\n", lenweek, lineout, day_headings);
  }

  // if(julian) printf(1, "julian\n");
  for (row = 0; row < 6; row++)
  {
    for (col = 0, p = lineout; col < 7; col++, p += (julian ? J_DAY_LEN: DAY_LEN))
    {
      ascii_day(p, days[row * 7 + col]);
    }
    *p = 0;
    trim_trailing_spaces(lineout);
    printf(1, "%s\n", lineout);
  }
}

void j_yearly(int year)
{
  int col, *dp,month, row, which_cal;
  char *p;
  int days[12][MAXDAYS];
  char lineout[80];

  // (void)sprintf(lineout, "%d", year);

  char num[10];
  int count = 0;
  int use = year;
  while (use != 0)
  {
    use /= 10; // angka = angka/10
    count++;
  }
  intToStr(year, num, count);
  strcpy(lineout, num);

  center(lineout, J_WEEK_LEN * 2 + J_HEAD_SEP, 0);
  printf(1, "\n\n");
  
  printf(1,"j_yearly:\tyear = %d\n", year);
  day_array(1, year, days[0]);
  day_array(2, year, days[1]);
  day_array(3, year, days[2]);
  day_array(4, year, days[3]);
  day_array(5, year, days[4]);
  day_array(6, year, days[5]);
  day_array(7, year, days[6]);
  day_array(8, year, days[7]);
  day_array(9, year, days[8]);
  day_array(10, year, days[9]);
  day_array(11, year, days[10]);
  day_array(12, year, days[11]);

  
  memset(lineout, ' ', sizeof(lineout) - 1);
  lineout[sizeof(lineout) - 1] = '\0';
  for (month = 0; month < 12; month += 2)
  {
    center(month_names[month], J_WEEK_LEN, J_HEAD_SEP);
    center(month_names[month + 1], J_WEEK_LEN, 0);

    char headsep[10];
    strcpy(headsep, " ");
    for (int i = 1; i < HEAD_SEP; i++)
    {
      strcat(headsep, " ");
    }

    printf(1, "\n%s%s%s\n", j_day_headings, headsep, j_day_headings);

    for (row = 0; row < 6; row++)
    {
      for (which_cal = 0; which_cal < 2; which_cal++)
      {
        p = lineout + which_cal * (J_WEEK_LEN + 2);
        dp = &days[month + which_cal][row * 7];
        for (col = 0; col < 7; col++, p += J_DAY_LEN)
          ascii_day(p, *dp++);
      }
      trim_trailing_spaces(lineout);
      printf(1, "%s\n", lineout);
    }
  }
  printf(1, "\n");
}

void yearly(int year)
{
  int col, *dp, month, row, which_cal;
  char *p;
  int days[12][MAXDAYS];
  char lineout[80];

  // (void)sprintf(lineout, "%d", year);

  char num[10];
  int count = 0;
  int use = year;
  while (use != 0)
  {
    use /= 10; // angka = angka/10
    count++;
  }
  intToStr(year, num, count);
  strcpy(lineout, num);

  center(lineout, (WEEK_LEN*3) + (HEAD_SEP*2), 0);
  printf(1, "\n\n");
  
  
  printf(1,"yearly:\tyear = %d\n", year);
 
  day_array(1, year, days[0]);
  day_array(2, year, days[1]);
  day_array(3, year, days[2]);
  day_array(4, year, days[3]);
  day_array(5, year, days[4]);
  day_array(6, year, days[5]);
  day_array(7, year, days[6]);
  day_array(8, year, days[7]);
  day_array(9, year, days[8]);
  day_array(10, year, days[9]);
  day_array(11, year, days[10]);
  day_array(12, year, days[11]);
  
  memset(lineout, ' ', sizeof(lineout) - 1);
  lineout[sizeof(lineout) - 1] = '\0';
  for (month = 0; month < 12; month += 3)
  {
    center(month_names[month], WEEK_LEN, HEAD_SEP);
    center(month_names[month + 1], WEEK_LEN, HEAD_SEP);
    center(month_names[month + 2], WEEK_LEN, 0);

    char headsep[10];
    strcpy(headsep, " ");
    for (int i = 1; i < HEAD_SEP; i++)
    {
      strcat(headsep, " ");
    }

    printf(1, "\n%s%s%s%s%s\n", day_headings, headsep, day_headings, headsep, day_headings);

    for (row = 0; row < 6; row++)
    {
      for (which_cal = 0; which_cal < 3; which_cal++)
      {
        p = lineout + which_cal * (WEEK_LEN + 2);
        dp = &days[month + which_cal][row * 7];
        for (col = 0; col < 7; col++, p += DAY_LEN)
          ascii_day(p, *dp++);
      }
      trim_trailing_spaces(lineout);
      printf(1, "%s\n", lineout);
    }
  }
  printf(1, "\n");
}

void usage()
{
  printf(2, "usage: cal [-jy] [[month] year]\n");
  exit();
}

int main(int argc, char **argv)
{
  printf(1, "argc = %d\n", argc);
  struct rtcdate r;
  if (date(&r))
  {
    printf(2, "date failed\n");
    exit();
  }

  int month, year, yflag;

  yflag = 0;
  int execLen = strlen(argv[0]);
  printf(1, "exec = %s\texeclen = %d\n", argv[0], execLen);

  int arCounter = argc - 1;
  int nonOptCnt = 0;
  char **startArgv = argv + argc;
  // printf(1, "start = %s\n", argv[1]+1);

  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] != '-')
    {
      printf(1, "\n\nargv[%d][0] = %c\n", i, argv[i][0]);
      printf(1, "not a option\n");
      nonOptCnt++;
      startArgv--;
      continue;
    }
    int len = strlen(argv[i]);
    printf(1, "\n\nargv[%d] = %s\tlen = %d\n", i, argv[i], len);

    char temp[len];
    printf(1, "debug %s\n", argv[i]+1);
    strcpy(temp, argv[i]+1);
    printf(1, "temp = %s\n", temp);

    int lentemp = strlen(temp);
    printf(1, "lentemp = %d\n", lentemp);

    for (int i = 0; i < lentemp; i++)
    {
      char tempchar;
      tempchar = temp[i];
      printf(1, "ch = %c\n", tempchar);
      switch (tempchar)
      {
      case 'j':
        julian = 1;
        break;
      case 'y':
        yflag = 1;
        break;
      default:
        usage();
      }
    }
    arCounter--;
  }

  printf(1, "*argv = %s\targc = %d\t*startArgv = %s\tnonOptCnt = %d\n\n", *argv, argc, *startArgv, nonOptCnt);

  month = 0;
  switch (nonOptCnt) //nonOptCnt
  {
  case 2:
    printf(1, "*startArgv  = %s\n", *startArgv);
    if ((month = atoi(*startArgv++)) <= 0 || month > 12)
    {
      printf(2, "cal: illegal month value: use 0-12\n");
      exit();
    }
    /* FALLTHROUGH */
  case 1:
    if ((year = atoi(*startArgv)) <= 0 || year > 9999)
    {
      printf(2, "cal: illegal year value: use 0-9999\n");
      exit();
    }
    break;
  case 0:
    year = r.year;
    if (!yflag) month = r.month;
    break;
  default:
    usage();
  }

  if (month) monthly(month, year);
  else if (julian) j_yearly(year);
  else yearly(year);
  exit();
}