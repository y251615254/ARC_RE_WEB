/*----------------------------------------------------------------------
 *+
 *  Compute_Day
 *  Compute Day of Week
 *
 *  Usage
 *
 *      int
 *      Compute_Day(date, month, year)
 *      int     date ;
 *      int     month ;
 *      int     year ;
 *
 *  Parameters
 *
 *      date            Date of Month (1 - 31)
 *      month           Month of Year (1 - 12)
 *      year            Year (e.g., 1989)
 *
 *  Description
 *
 *      Compute_Day() computes the day of the week for the given date
 *      using the Zeller Congruence.  It returns a positive value from
 *      0 to 6 for the day of the week
 *
 *  Notes
 *
 *      The Zeller Congruence maps Saturday as day 0.  Most applications
 *      treat Sunday as day 0.  The parameter ZELLER_OFFSET is used to
 *      convert the day of the week from Zeller to local.
 *
 *-
 */

//#define WEEKDAY_TEST
//#define _NET186
#define _186CU

#ifdef _NET186
static char *_weekdays[] = {
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
	"Sunday"
};
static char *_wkdays[] = {
	"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};
#elif defined(_186CU)
static char *_weekdays[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};
static char *_wkdays[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
#else
static char *_weekdays[] = {
	"Saturday",
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday"
};
static char *_wkdays[] = {
	"Sat", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri"
};
#endif

static int
Compute_Day(int date, int month, int year)
{
	int     day ;
	int     yr ;
	int     mn ;
	int     n1 ;
	int     n2 ;

	/* Offset from Zeller to local */
	/* --------------------------- */
#ifdef _NET186
#define ZELLER_OFFSET	5	/* Monday:0,... , Sunday: 6 */
#elif defined(_186CU)
#define ZELLER_OFFSET	6	/* Sunday: 0,... ,Saturday: 6 */
#else 
#define ZELLER_OFFSET   0	/* Saturday:0, ..., Friday: 6 */
#endif

	yr = year ;
	mn = month ;

	/* January or February? */
	/* -------------------- */
	if (mn < 3)
	{
		/* Yes, make these part of last year */
		/* --------------------------------- */
		mn += 12 ;
		yr -= 1 ;
	}

	n1 = (26 * (mn + 1)) / 10 ;
	n2 = (int) ((125 * (long) yr) / 100) ;

	day = ((date + n1 + n2 - (yr / 100) + (yr / 400) + ZELLER_OFFSET) % 7) ;
	return day ;
}

char *weekday(int year, int mon, int day) {
	return _weekdays[Compute_Day(day, mon, year)];
}

char *wkday(int year, int mon, int day) {
	return _wkdays[Compute_Day(day, mon, year)];
}

char *get_wkday(int idx) {
	return _wkdays[(idx+1) % 7];
}

#ifdef WEEKDAY_TEST
main() {
        printf("%d/%d/%d: %d\n", 6, 1, 1998, Compute_Day(1, 6, 1998)); // 1
        printf("%d/%d/%d: %d\n", 11, 2, 1999, Compute_Day(2, 11, 1999)); // 2
        printf("%d/%d/%d: %d\n", 3, 3, 1999, Compute_Day(3, 3, 1999)); // 3
        printf("%d/%d/%d: %d\n", 7, 1, 1999, Compute_Day(1, 7, 1999)); // 4
        printf("%d/%d/%d: %d\n", 9, 18, 1998, Compute_Day(18, 9, 1998)); // 5
        printf("%d/%d/%d: %d\n", 3, 27, 1999, Compute_Day(27, 3, 1999)); // 6
        printf("%d/%d/%d: %d\n", 6, 13, 1999, Compute_Day(13, 6, 1999)); // 7

        return 0;
}
#endif
