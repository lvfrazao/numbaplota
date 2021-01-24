#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#define INIT_BUF 4096
#define BUF_GROWTH_FACTOR 2
#define COL 80
#define ROW 40

typedef struct
{
    double x; // x coordinate of point
    double y; // y coordinate of point
} Point;

typedef struct
{
    Point *points; // Array of point structs
    int len;       // Number of elements in the points member
} PointList;

typedef struct
{
    Point min; // Minimum x and y
    Point max; // Maximum x and y
} Bounds;

PointList *read_input(FILE *f);
Point line2point(char *line, char delimiter);
void print_points(PointList *pl);
void print_point(Point p);
double str2double(char *line);
char *strip(char *str, char c);
Bounds get_axis_bounds(PointList *pl);
int check_xpoints_between(PointList *pl, double lower, double upper);
void get_ypoints_between(PointList *pl, PointList *ret_pl, double lower, double upper);
int num_digits(double num);
void clear_str(char *str, int len);

int main(int argc, char **argv)
{
    char *usage =
        "Usage: numbaplota\n"
        "Positional Arguments:\n"
        "    filename:    Name of file to read from, alternately reads from stdin\n";
    // TODO: Add option for setting different delimiters
    // TODO: Add option for setting graph size (maybe detect term size?)
    PointList *points;
    FILE *f;
    switch (argc)
    {
    case 1:
        // Read from stdin
        points = read_input(stdin);
        break;
    case 2:
        // Read from file
        f = fopen(argv[1], "r");
        if (f == NULL)
        {
            perror("Error opening input file");
            exit(EXIT_FAILURE);
        }
        points = read_input(f);
        fclose(f);
        break;
    default:
        fprintf(stderr, "Invalid options\n%s", usage);
        return EXIT_FAILURE;
    }

#ifdef DEBUG
    printf("%s: Loaded points\n", __func__);
    print_points(points);
#endif

    Bounds bounds = get_axis_bounds(points);
    double xspan, yspan, incx, incy;
    xspan = bounds.max.x - bounds.min.x;
    yspan = bounds.max.y - bounds.min.y;
    incx = xspan / (COL - 2);
    incy = yspan / (ROW - 2);
#ifdef DEBUG
    printf("%s: xspan=%f yspan=%f incx=%f incy=%f\n", __func__, xspan, yspan, incx, incy);
#endif
    // Axis bounds will be one more than max and one less than min
    double maxX, maxY, minX, minY;
    maxX = bounds.max.x + incx;
    maxY = bounds.max.y + incy;
    minX = bounds.min.x - incx;
    minY = bounds.min.y - incy;

    // Set up buffer to get all points between y bounds
    PointList *pl_in_ybounds = malloc(sizeof(PointList));
    // Yolo I don't want to deal with dynamically reallocating this array
    // We can optimize this later if its a big deal
    Point *ret_points = malloc(sizeof(Point) * points->len);
    pl_in_ybounds->points = ret_points;

    // Max value for a double is 1.7E+308 which means the integer portion will have a max of 309 digits
    // The+ 4 is to account for the period and single value after the period
    char *maxY_str = calloc(num_digits(maxY) + 4, sizeof(char));
    char *minY_str = calloc(num_digits(minY) + 4, sizeof(char));
    // TODO: support more dynamic setting of float precision, maybe make precision based on the yspan
    snprintf(maxY_str, num_digits(maxY) + 4, "%.1f", maxY);
    snprintf(minY_str, num_digits(minY) + 4, "%.1f", minY);
    char marker = '*';
    int len_max = strlen(maxY_str) > strlen(minY_str) ? strlen(maxY_str) : strlen(minY_str);
    free(maxY_str);
    free(minY_str);
    for (double cur_upper_y = maxY; cur_upper_y > minY; cur_upper_y -= incy)
    {
        printf("%*.1f |", len_max, cur_upper_y);
        get_ypoints_between(points, pl_in_ybounds, cur_upper_y - incy, cur_upper_y);
        for (double cur_lower_x = minX; cur_lower_x < maxX; cur_lower_x += incx)
        {
            if (check_xpoints_between(pl_in_ybounds, cur_lower_x, cur_lower_x + incx))
                printf("%c", marker);
            else
                printf(" ");
        }
        printf("\n");
    }

    // Construct the X axis
    // Max value for a double is 1.7E+308 which means the integer portion will have a max of 309 digits
    char *maxX_str = calloc(num_digits(maxX) + 5, sizeof(char));
    char *minX_str = calloc(num_digits(minX) + 5, sizeof(char));
    snprintf(maxX_str, num_digits(maxX) + 5, "_%.1f", maxY);
    snprintf(minX_str, num_digits(minX) + 5, "_%.1f", minY);
    int len_max_x = strlen(maxX_str) > strlen(minX_str) ? strlen(maxX_str) : strlen(minX_str);
    free(maxX_str);
    free(minX_str);
    char *xaxis_str = calloc(len_max_x, sizeof(char));

    for (int row = 0; row < len_max_x; row++)
    {
        for (int i = 0; i < len_max; i++){
            printf(" ");
        }
        printf("  ");
        int col_num = 0;
        for (double cur_lower_x = minX; cur_lower_x < maxX; cur_lower_x += incx)
        {
            col_num++;
            if (col_num % 2 == 0) {
                printf(" ");
                continue;
            }
            snprintf(xaxis_str, len_max_x, "_%.1f", cur_lower_x);
            if (xaxis_str[row] == '\0')
                printf(" ");
            else
                printf("%c", xaxis_str[row]);
            clear_str(xaxis_str, len_max_x);
        }
        printf("\n");
    }

    // It doesnt really matter, but I want to free everything before leaving
    free(xaxis_str);
    free(ret_points);
    free(pl_in_ybounds);
    free(points->points);
    free(points);
    return 0;
}

void clear_str(char *str, int len)
{
    for (int i = 0; i < len; i++)
    {
        str[i] = '\0';
    }
}

int num_digits(double num)
{
    int count;
    for (count = 1; num > 10; count++, num /= 10)
    {
    }
#ifdef DEBUG
    printf("%s: num digits in %f is %d\n", __func__, num, count);
#endif
    return count;
}

void get_ypoints_between(PointList *pl, PointList *ret_pl, double lower, double upper)
{
    // Place all points between a lower and upper Y bounds in ret_pl
    ret_pl->len = 0;

    for (int i = 0; i < pl->len; i++)
    {
        if (pl->points[i].y > lower && pl->points[i].y <= upper)
        {
            ret_pl->points[ret_pl->len] = pl->points[i];
            ret_pl->len++;
#ifdef DEBUG
            printf("%s: found point between %f and %f - (%f, %f)\n", __func__, lower, upper, pl->points[i].x, pl->points[i].y);
#endif
        }
    }
}

int check_xpoints_between(PointList *pl, double lower, double upper)
{
    // Return 1 if any point in pl is between given x bounds
    for (int i = 0; i < pl->len; i++)
    {
        if (pl->points[i].x >= lower && pl->points[i].x < upper)
#ifdef DEBUG
            printf("%s: found point between %f and %f - (%f, %f)\n", __func__, lower, upper, pl->points[i].x, pl->points[i].y);
#endif
        return 1;
    }
    return 0;
}

Bounds get_axis_bounds(PointList *pl)
{
    Bounds b = {
        .min = pl->points[0],
        .max = pl->points[0]};
    for (int i = 1; i < pl->len; i++)
    {
        b.min.x = (pl->points[i].x < b.min.x) ? pl->points[i].x : b.min.x;
        b.min.y = (pl->points[i].y < b.min.y) ? pl->points[i].y : b.min.y;
        b.max.x = (pl->points[i].x > b.max.x) ? pl->points[i].x : b.max.x;
        b.max.y = (pl->points[i].y > b.max.y) ? pl->points[i].y : b.max.y;
    }

#ifdef DEBUG
    printf("%s: bounds found (%f, %f) and (%f, %f)\n", __func__, b.min.x, b.min.y, b.max.x, b.max.y);
#endif

    return b;
}

PointList *read_input(FILE *f)
{
    char DELIMITER = ' ';
    char *line = NULL;
    size_t len = 0;
    ssize_t nread = 0;
    int buf_size = INIT_BUF;

    PointList *pl = malloc(sizeof(PointList));
    Point *p = malloc(sizeof(Point) * buf_size);
    if (p == NULL || pl == NULL)
    {
        perror("Failed to allocate buffer for data");
        exit(EXIT_FAILURE);
    }
    pl->points = p;
    pl->len = 0;

    while ((nread = getline(&line, &len, f)) != -1)
    {
        // Get the line, parse the columns, store as signed ints
        if (pl->len + 1 > buf_size)
        {
            // Data buffer is full, must reallocate
            int new_buf_size = buf_size * BUF_GROWTH_FACTOR;
#ifdef DEBUG
            printf("%s: Data buffer is full, must reallocate, pl->len=%d buf_size=%d\n", __func__, pl->len, buf_size);
            printf("%s: New buffer size will be %d\n", __func__, new_buf_size);
#endif
            if ((pl->points = reallocarray(pl->points, new_buf_size, sizeof(Point))) == NULL)
            {
                perror("Failed to reallocate data buffer");
                exit(EXIT_FAILURE);
            }
            buf_size = new_buf_size;
        }
        if (nread > 1)
        {
            line[nread - 1] = '\0';
            pl->points[pl->len] = line2point(line, DELIMITER);
            pl->len++;
        }
    }
    free(line);

    return pl;
}

Point line2point(char *line, char delimiter)
{
    /* 
    Takes a line and a delimiter and returns a point.

    e.g., for the values of line="1 2", delimiter=" "
    The point returned is: {}
    */
    static int count = 0;
    char *first_col_start = line;
    int two_columns = 0;
    double valx, valy;
    line = strip(line, delimiter);
    while (*line != '\0')
    {
        if (*line == delimiter)
        {
            *line = '\0';
            two_columns = 1;
            valy = str2double(line + 1);
            break;
        }
        line++;
    }
    valx = str2double(first_col_start);
    if (!two_columns)
    {
        valy = valx;
        valx = count;
    }

    Point p = {
        .x = valx,
        .y = valy,
    };
    count++;
    return p;
}

void print_points(PointList *pl)
{
    for (int i = 0; i < pl->len; i++)
    {
        print_point(pl->points[i]);
    }
}

void print_point(Point p)
{
    printf("(%f, %f)\n", p.x, p.y);
}

double str2double(char *line)
{
    char *end_ptr;
    double value = strtod(line, &end_ptr);
    if (line == end_ptr)
    {
#ifdef DEBUG
        printf("%s: Encountered an invalid value: %s\n", __func__, line);
#endif
        // Not a floating point value, ignore
        return 0;
    }
    return value;
}

char *strip(char *str, char c)
{
    // Strips the ends of a string of char c
    // e.g., strip spaces from beginning and end

    // strip the beginning
    while (*str == c)
    {
        str++;
    }

    // strip the ending
    int str_len = strlen(str);
    int i = 1;
    while (str[str_len - i] == c && i <= str_len)
    {
        str[str_len - i] = '\0';
        i++;
    }
    return str;
}
