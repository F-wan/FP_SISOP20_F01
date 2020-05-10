#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "date.h"

#define ENTRY_SIZE 3      /* 3 bytes per value		 */
#define DAYS_PER_WEEK 7   /* Sunday, etc.			 */
#define WEEKS_PER_MONTH 6 /* Max. weeks in a month	 */
#define MONTHS_PER_LINE 3 /* Three months across		 */
#define MONTH_SPACE 3     /* Between each month		 */

char *badarg = {"Bad argument\n"};
char *how = {"Usage: cal [month] year\n"};

char layout[MONTHS_PER_LINE][WEEKS_PER_MONTH][DAYS_PER_WEEK][ENTRY_SIZE];
char outline[(MONTHS_PER_LINE * DAYS_PER_WEEK * ENTRY_SIZE) + (MONTHS_PER_LINE * MONTH_SPACE) + 1];

char *weekday = " S  M Tu  W Th  F  S";
char *monthname[] = {
    "???", /* No month 0	 */
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

struct
{
  int this_month;    /* month number used in 1752 checking	 */
  int feb;           /* Days in February for this month	 */
  int sept;          /* Days in September for this month	 */
  int days_in_month; /* Number of days in this month		 */
  int dow_first;     /* Day of week of the 1st day in month	 */
} info;

int day_month[] = {/* 30 days hath September...		 */
                          0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int Jan1(int year)
{
  int day;

  day = year + 4 + ((year + 3) / 4); /* Julian Calendar	 */
  if (year > 1800)
  {                               /* If it's recent, do	 */
    day -= ((year - 1701) / 100); /* Clavian correction	 */
    day += ((year - 1601) / 400); /* Gregorian correction	 */
  }
  if (year > 1752) day += 3;      /* calendar		 */
  return (day % 7);
}


void setmonth(int year, int month)
{
  int i;

  if (month < 1 || month > 12)
  {                         /* Verify caller's parameters	 */
    info.days_in_month = 0; /* Garbage flag			 */
    return;
  }
  info.this_month = month;     /* used in 1752	checking	 */
  info.dow_first = Jan1(year); /* Day of January 1st for now	 */
  info.feb = 29;               /* Assume leap year		 */
  info.sept = 30;              /* Assume normal year		 */
  /* Determine whether it's an ordinary year, a leap year or the
   * magical calendar switch year of 1752. */
  switch ((Jan1(year + 1) + 7 - info.dow_first) % 7)
  {
  case 1: /* Not a leap year		 */
    info.feb = 28;
  case 2: /* Ordinary leap year		 */
    break;

  default:          /* The magical moment arrives	 */
    info.sept = 19; /* 19 days hath September	 */
    break;
  }
  info.days_in_month =
      (month == 2) ? info.feb
                   : (month == 9) ? info.sept
                                  : day_month[month];
  for (i = 1; i < month; i++)
  {
    switch (i)
    {       /* Special months?		 */
    case 2: /* February			 */
      info.dow_first += info.feb;
      break;

    case 9:
      info.dow_first += info.sept;
      break;

    default:
      info.dow_first += day_month[i];
      break;
    }
  }
  info.dow_first %= 7; /* Now it's Sunday to Saturday	 */
}

int getdate(int week, int wday)
{
  int today;

  /* Get a first guess at today's date and make sure it's in range. */
  today = (week * 7) + wday - info.dow_first + 1;
  if (today <= 0 || today > info.days_in_month)
    return (0);
  else if (info.sept == 19 && info.this_month == 9 && today >= 3) /* The magical month?	 */
    return (today + 11);                                          /* If so, some dates changed	 */
  else                                                            /* Otherwise,			 */
    return (today);                                               /* Return the date		 */
}


void calendar(int year, int month, int indx)
{
  char *tp;
  int week;
  int wday;
  int today;

  setmonth(year, month);
  for (week = 0; week < WEEKS_PER_MONTH; week++)
  {
    for (wday = 0; wday < DAYS_PER_WEEK; wday++)
    {
      tp = &layout[indx][week][wday][0];
      *tp++ = ' ';
      today = getdate(week, wday);
      if (today <= 0)
      {
        *tp++ = ' ';
        *tp++ = ' ';
      }
      else if (today < 10)
      {
        *tp++ = ' ';
        *tp = (today + '0');
      }
      else
      {
        *tp++ = (today / 10) + '0';
        *tp = (today % 10) + '0';
      }
    }
  }
}

void usage(s) char *s;
{
  printf(2, "%s", s);
  exit();
}

void output(int nmonths)
{
  int week;
  int month;
  char *outp;
  int i;
  char tmpbuf[21], *p;

  for (week = 0; week < WEEKS_PER_MONTH; week++)
  {
    outp = outline;
    for (month = 0; month < nmonths; month++)
    {
      /* The -1 in the following removes the unwanted
		 * leading blank from the entry for Sunday. */
      p = &layout[month][week][0][1];
      for (i = 0; i < 20; i++)
        tmpbuf[i] = *p++;
      tmpbuf[20] = 0;
  
      // sprintf(outp, "%s   ", tmpbuf);
      strcpy(outp, tmpbuf);
      strcat(outp, "   ");
      outp += (DAYS_PER_WEEK * ENTRY_SIZE) + MONTH_SPACE - 1;
    }
    while (outp > outline && outp[-1] == ' ')
      outp--;
    *outp = 0;
    printf(1,"%s\n", outline);
  }
}


void doyear(year) int year;
/* Print the calendar for an entire year. */
{
  int month;

  if (year < 1 || year > 9999)
    usage(badarg);
  if (year < 100)
    printf(1,"\n\n\n                                 00%2d\n\n", year);
  else
    printf(1,"\n\n\n                                 %d\n\n", year);
  for (month = 1; month <= 12; month += MONTHS_PER_LINE)
  {
    printf(1,"\t%s\t\t\t%s\t\t\t%s\n",
           monthname[month],
           monthname[month + 1],
           monthname[month + 2]);
    printf(1,"%s   %s   %s\n", weekday, weekday, weekday);
    calendar(year, month + 0, 0);
    calendar(year, month + 1, 1);
    calendar(year, month + 2, 2);
    output(3);
#if MONTHS_PER_LINE != 3
#error "the above will not work"
#endif
  }
  printf(1,"\n\n\n");
}

void domonth(year, month) int year;
int month;
/* Do one specific month -- note: no longer used */
{
  if (year < 1 || year > 9999)
    usage(badarg);
  if (month <= 0 || month > 12)
    usage(badarg);
  printf(1,"         %s     %d\n\n%s\n", monthname[month], year, weekday);
  calendar(year, month, 0);
  output(1);
  printf(1,"\n\n");
}

int main(int argc, char **argv)
{
  struct rtcdate r;
  if (date(&r))
  {
    printf(2, "date failed\n");
    exit();
  }
  int year, month;
 
  year = r.year;
  month = r.month;

  int arg1val;
  int arg1len;
  int arg2val;

  if (argc <= 1)
  {
    domonth(year, month);
  }
  else
  {
    arg1val = atoi(argv[1]);
    arg1len = strlen(argv[1]);
    if (argc == 2)
    {
      if (arg1len <= 2 && arg1val <= 12)
        domonth(year, arg1val);
      else
        doyear(arg1val);
    }
    else
    {
      arg2val = atoi(argv[2]);
      if (arg1len > 2)
        domonth(arg1val, arg2val);
      else
        domonth(arg2val, arg1val);
    }
  }
  exit();
}
