/*This is program is an IGuhit Language Interpreter. It interpretes the various*/
/*IGuhit commands from an input text file and would save it to a bitmap file   */
/*when directed.*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*Constants in Interpreter*/

#define POINT_MAX 100 /*Maximum number of points that can be created*/
#define BOX_MAX 10 /*Maximum number of boxes that can be created*/
#define CIRCLE_MAX 10 /*Maximum number of circles that can be created*/
#define LINE_MAX 10 /*Maximum number of lines that can be created*/
#define CHAR_MAX 1024 /*Maximum length of string*/
#define PARAM_MAX 6 /*Maximum length of command*/
#define PIXEL_MAX 200 /*Maximim number of pixels*/
#define RADIUS_MAX 90 /*Maximum radius*/

/*Error flags*/

#define NO_ERROR 2
#define ERR_CREATEFILE 3
#define ERR_HEADER 4
#define ERR_POINTMAX 5
#define ERR_POINT 6
#define ERR_LINEMAX 7
#define ERR_LINE 8
#define ERR_BOXMAX 9
#define ERR_BOX 10
#define ERR_BOXCORNER 11
#define ERR_CIRCLEMAX 12
#define ERR_RADIUSMAX 13
#define ERR_CENTER 14
#define ERR_MOVPT 15
#define ERR_MOVSHAPE 16

/*Structures for objects that can be created using IGuhit*/

typedef struct {
  int centerx;
  int centery;
  int radius;
  int occupied;
  char color;
} circle, *circle_ptr;

typedef struct {
  int line1x;
  int line1y;
  int line2x;
  int line2y;
  int occupied;
  char color;
} line, *line_ptr;

typedef struct {
  int top_leftx;
  int top_lefty;
  int bot_rightx;
  int bot_righty;
  int occupied;
  char color;
} box, * box_ptr;

typedef struct {
  int x;
  int y;
  int occupied;
  char color;
} point, *point_ptr;

typedef struct {
  point pt[POINT_MAX];
} pts, *pts_ptr;

typedef struct {
  line lines[LINE_MAX];
} ln, *ln_ptr;

typedef struct {
  box boxes[BOX_MAX];
} bx, *bx_ptr;

typedef struct {
  circle circles[CIRCLE_MAX];
} cir, *cir_ptr;

typedef struct {
  float data[200];
  char color;
  int occupied;
} graph, *graph_ptr;

/*Function Prototypes*/

int BMPheader (FILE *fp);
int round_off (double entry);
double compute_slope (int x1, int y1, int x2, int y2);
int compute_diff(int, int);
void clean_ws (char []);
void rem_trail_ws (char []);
void process_cmd (char [], char [][CHAR_MAX]);
int process (char [][CHAR_MAX], pts_ptr ps, ln_ptr ls, bx_ptr bs, cir_ptr cs, graph_ptr g);
void initialize_pts (pts_ptr ps);
void initialize_ln (ln_ptr ls);
void initialize_bx (bx_ptr bs);
void initialize_cir (cir_ptr cs);
void initialize_graph (graph_ptr g);
int save_work (char [][CHAR_MAX], pts_ptr ps, ln_ptr ls, bx_ptr bs, cir_ptr cs, graph_ptr g);
void print_error (int);
int load_point (char [][CHAR_MAX], pts_ptr ps);
int load_line (char [][CHAR_MAX], ln_ptr ls);
int load_box (char [][CHAR_MAX], bx_ptr bs);
int load_circle (char [][CHAR_MAX], cir_ptr cs);
int load_graph (char [][CHAR_MAX], graph_ptr g);
void delete_point (char [][CHAR_MAX], pts_ptr ps);
void delete_line (char [][CHAR_MAX], ln_ptr ls);
void delete_box (char [][CHAR_MAX], bx_ptr bs);
void delete_circle (char[][CHAR_MAX], cir_ptr cs);
int move_point (char [][CHAR_MAX], pts_ptr ps);
int move_line (char [][CHAR_MAX], ln_ptr ls);
int move_box (char [][CHAR_MAX], bx_ptr bs);
int move_circle (char [][CHAR_MAX], cir_ptr cs);
void create_graph (char [][PIXEL_MAX], graph_ptr g);
void compute_midpt (int*, int*, int, int, int, int);
void create_point (char [][PIXEL_MAX], pts_ptr ps);
void create_line (char [][PIXEL_MAX], ln_ptr ls);
void create_box (char [][PIXEL_MAX], bx_ptr bs);
void create_circle (char [][PIXEL_MAX], cir_ptr cs);

/*MAIN FUNCTION*/

int main (int argc, char **argv)
{
  FILE *infile;
  char string[CHAR_MAX], cmd[PARAM_MAX][CHAR_MAX];
  int error = NO_ERROR, count, count2;
  pts points;
  ln lines;
  bx boxes;
  cir circles;
  graph grap;

 
  initialize_pts (&points);
  initialize_ln (&lines);
  initialize_bx (&boxes);
  initialize_cir (&circles);
  initialize_graph (&grap);

  if (argc < 2) 
  {
     printf ("ERROR: Input file not specified!\n");
     return 1;
  }

  infile = fopen(argv[1], "r");
  while (!feof(infile))
  {
    for (count = 0; count < CHAR_MAX; count++) string[count] = '\0';

    for (count = 0; count < PARAM_MAX; count++)
      {
	for (count2 = 0; count2 < CHAR_MAX; count2++)
	  {
	    cmd[count][count2] = '\0';
	  }
      }
     fgets(string, CHAR_MAX, infile);
     clean_ws(string);
     rem_trail_ws (string);
     if (strlen(string) != 0)
       {
	 process_cmd (string, cmd);
	 error = process (cmd, &points, &lines, &boxes, &circles, &grap);
	 if (error != NO_ERROR)
	   {
	     print_error(error);
	     fclose(infile);
	     return 1;
	   }
       }
  }
  fclose (infile);
  return 0;
}

/*The following functions initialize the subjects that will contain*/
/*that are going to be created in Iguhit*/ 

void initialize_pts (pts_ptr ps)
{
  memset(ps, 0, POINT_MAX * sizeof(point));
}

void initialize_ln (ln_ptr ls)
{
  memset(ls, 0, LINE_MAX * sizeof(line));
}

void initialize_bx (bx_ptr bs)
{
  memset(bs, 0, BOX_MAX * sizeof(box));
}

void initialize_cir (cir_ptr cs)
{
  memset(cs, 0, CIRCLE_MAX * sizeof(circle));
}

void initialize_graph (graph_ptr g)
{
  memset(g->data, 0, PIXEL_MAX * sizeof(float));
  g->color = 0;
  g->occupied = 0;
}

/*clean_ws removes any leading whitespace in string after using fgets*/

void clean_ws(char string[])
{
  char temp[1024];
  int count, count2;

  count2 = 0;
  for (count = 0; count < strlen(string); count++)
  {
    if (!isspace(string[count])) break;
  }

  for (; count < strlen(string); count++)
  {
	  temp[count2] = string[count];
	  count2++;
	  temp[count2] = '\0';
  }
  strcpy(string, temp);
}

/*rem_trail_ws removes any trailing whitespace like \n at end of string after*/
/*using fgets*/

void rem_trail_ws (char line [])
{
  char c;
  int j = strlen(line)-1;
  int count;

  if (strcmp(line, "\n") == 0) line[0] = '\0';
  else {
    for (count = j; count >= 0; count--)
      {
	if (!isspace(line[count]))
	  {
	    count++;
	    line[count] = '\0';
	    break;
	  }
      }
  }
}

/*process analyzes command line and calls on the function for the corresponding*/
/*command*/

int process (char cmd[][CHAR_MAX], pts_ptr ps, ln_ptr ls, bx_ptr bs, cir_ptr cs, graph_ptr g)
{
  int error = NO_ERROR;

  switch (cmd[0][0])
    {
    case 'P':
      error = load_point (cmd, ps);
      break;
     
    case 'L':
      error = load_line (cmd, ls);
      break;
    
    case 'B':
      error = load_box (cmd, bs);
      break;
 
    case 'C':
      error = load_circle (cmd, cs);
      break;
    }
  
  if (strcmp(cmd[0], "MOVE") == 0)
    {
      switch (cmd[1][0])
        {
	case 'P':
	  error = move_point (cmd, ps);
	  break;
     
	case 'L':
	  error = move_line (cmd, ls);
	  break;
    
	case 'B':
	  error = move_box (cmd, bs);
	  break;
 
	case 'C':
	  error = move_circle (cmd, cs);
	  break;
        }

    }

  if (strcmp(cmd[0], "DELT") == 0)
    {
      switch (cmd[1][0])
        {
	case 'P':
	  delete_point (cmd, ps);
	  break;
     
	case 'L':
	  delete_line (cmd, ls);
	  break;
    
	case 'B':
	  delete_box (cmd, bs);
	  break;
 
	case 'C':
	  delete_circle (cmd, cs);
	  break;
        }
    }
  
  if (strcmp(cmd[0], "GRAP") == 0) error = load_graph(cmd, g);

  if (strcmp(cmd[0], "SAVE") == 0) error = save_work(cmd, ps, ls, bs, cs, g);
  
  return error;  
}

/*process_cmd divides string obtained in input file into words for easy reading*/
/*places them to a two dimensional char array*/ 

void process_cmd (char string[], char cmd[][CHAR_MAX])
{

  char *marker, *marker2;
  int count, count2;

  marker = string;
  marker2 = strchr(string, ' ');

  count = 0;
  while (*marker != *marker2)
    {
      cmd[0][count] = *marker;
      count++;
      cmd[0][count] = '\0';
      marker++;
    }

  marker2 = strchr(marker, ',');
  if (marker2 == NULL)
    {
      marker++;
      count = 0;
      while (*marker != '\0')
	{
	  cmd[1][count] = *marker;
	  count++;
	  cmd[1][count] = '\0';
	  marker++;
	}
      clean_ws(cmd[1]);
    }
  else {
   
    count2 = 1;
    while ((*marker != '\0') && (count2 != PARAM_MAX))
      {
	marker++;
	marker2 = strchr(marker, ',');
	count = 0;
	if (marker2 != NULL)
	  {	  
	    while (*marker != *marker2)
	      {
		cmd[count2][count] = *marker;
		count++;
		cmd[count2][count] = '\0';
		marker++;
	      }
	  }
	else {
	  while (*marker != '\0')
	    {
	      cmd[count2][count] = *marker;
	      count++;
	      cmd[count2][count] = '\0';
	      marker++;
	    }
	}	  
	clean_ws(cmd[count2]);
	count2++;
      }
  }
}

/*load_point load information for a point to its structure and returns any*/
/*error flag if there is an error*/

int load_point (char cmd[][CHAR_MAX], pts_ptr ps)
{
  char *number;
  int pnum, colnum, pointx, pointy;

  number = cmd[0];
  number++;
  pnum = atoi(number) - 1;
  if ((pnum > -1) && (pnum < POINT_MAX))
    {
      pointx = atoi(cmd[1]);
      pointy = atoi(cmd[2]);
      if ((pointx < 1) || (pointy < 1) || (pointx > PIXEL_MAX) || (pointy > PIXEL_MAX)) 
	{
	  return ERR_POINT;
	}
      else
	{
	  ps->pt[pnum].x = atoi(cmd[1]);
	  ps->pt[pnum].y = atoi(cmd[2]);
	  ps->pt[pnum].occupied = 1;
	}
      colnum = atoi(cmd[3]);
      if ((colnum < 0) || (colnum > 4)) ps->pt[pnum].color = 1;
      else ps->pt[pnum].color = colnum;
      return NO_ERROR;
    }
  else return ERR_POINTMAX;
} 

/*load_line loads information for a line to its structure and returns an error */
/*flag if an error has occured*/

int load_line (char cmd[][CHAR_MAX], ln_ptr ls)
{
  int lnum, colnum, x1, x2, y1, y2;
  char *number;

  number = cmd[0];
  number++;
  lnum = atoi(number) - 1;

  if ((lnum > -1) && (lnum < LINE_MAX))
    {
      x1 = atoi(cmd[1]);
      y1 = atoi(cmd[2]);
      x2 = atoi(cmd[3]);
      y2 = atoi(cmd[4]);

      if ((x1 < 1) || (x2 < 1) || (x1 > PIXEL_MAX) || (x2 > PIXEL_MAX) || (y1 < 1) || (y2 < 1) || (y1 > PIXEL_MAX) || (y2 > PIXEL_MAX))
	{
	  return ERR_LINE;
	}
      else {
	ls->lines[lnum].line1x = x1;
	ls->lines[lnum].line1y = y1;
	ls->lines[lnum].line2x = x2;
	ls->lines[lnum].line2y = y2;
	ls->lines[lnum].occupied = 1;
	
      }

      colnum = atoi(cmd[5]);
      if ((colnum < 0) || (colnum > 4)) ls->lines[lnum].color = 1;
      else ls->lines[lnum].color = colnum;
      return NO_ERROR;
    }

  else return ERR_LINEMAX;      
}

/*load_box loads information for a box to its structure and returns an error */
/*flag if an error has occured*/

int load_box (char cmd[][CHAR_MAX], bx_ptr bs)
{
  int bnum, colnum, x1, y1, x2, y2;
  char *number;

  number = cmd[0];
  number++;
  bnum = atoi(number) - 1;

  if ((bnum > -1) && (bnum < BOX_MAX))
    {
      x1 = atoi(cmd[1]);
      y1 = atoi(cmd[2]);
      x2 = atoi(cmd[3]);
      y2 = atoi(cmd[4]);
      if ((x1 < 1) || (x2 < 1) || (x1 > PIXEL_MAX) || (x2 > PIXEL_MAX) || (y1 < 1) || (y2 < 1) || (y1 > PIXEL_MAX) || (y2 > PIXEL_MAX))
	{
	  return ERR_BOX;
	}
      else {
	if ((x2 < x1) || (y2 > y1)) return ERR_BOXCORNER;
	else {
	  bs->boxes[bnum].top_leftx = x1;
	  bs->boxes[bnum].top_lefty = y1;
	  bs->boxes[bnum].bot_rightx = x2;
	  bs->boxes[bnum].bot_righty = y2;
	  bs->boxes[bnum].occupied = 1;
	}
      }
      colnum = atoi(cmd[5]);
      if ((colnum < 0) || (colnum > 4)) bs->boxes[bnum].color = 1;
      else bs->boxes[bnum].color = colnum;
      return NO_ERROR;
    }

  else return ERR_BOXMAX;
}

/*load_circle loads information for a circle to its structure and returns an error*/
/*flag if an error has occured*/

int load_circle (char cmd[][CHAR_MAX], cir_ptr cs)
{
  int cnum, colnum, x, y, rad;
  char *number;

  number = cmd[0];
  number++;
  cnum = atoi(number) - 1;

  if ((cnum > -1) && (cnum < CIRCLE_MAX))
    {
      x = atoi(cmd[1]);
      y = atoi(cmd[2]);
      if ((x < 1) || (y < 1) || (x > PIXEL_MAX) || (y > PIXEL_MAX))
	{
	  return ERR_CENTER;
	}
      else {
	rad = atoi(cmd[3]);
	if ((rad < 0) || (rad > 89)) return ERR_RADIUSMAX;
	else {
	  cs->circles[cnum].centerx = x;
	  cs->circles[cnum].centery = y;
	  cs->circles[cnum].radius = rad;
	  cs->circles[cnum].occupied = 1;
	}
	colnum = atoi(cmd[4]);
	if ((colnum < 0) || (colnum > 4)) cs->circles[cnum].color = 1;
	else cs->circles[cnum].color = colnum;
	return NO_ERROR;
      }
    }

  else return ERR_CIRCLEMAX;
}

/*load_graph loads information for a graph to its structure and returns an error */
/*flag if an error has occured*/

int load_graph (char cmd[][CHAR_MAX], graph_ptr g)
{
  FILE *infile;
  int colnum;

  infile = fopen(cmd[1], "rb");
  fread(g->data, sizeof(float), PIXEL_MAX, infile);
  colnum = atoi(cmd[2]);
  if ((colnum < 0) || (colnum > 4)) g->color = 1;
  else g->color = colnum;
  g->occupied = 1;
  return NO_ERROR;
}

/*print_error prints out the specific error brought by the error flag*/

void print_error (int error)
{
  switch (error)
    {
    case ERR_CREATEFILE:
      printf ("ERROR: Output File can't be created.\n");
      break;

    case ERR_HEADER:
      printf ("ERROR: Can't create bitmap header.\n");
      break;

    case ERR_POINTMAX:
      printf ("ERROR: Invalid Point Number.\n");
      break;

    case ERR_POINT:
      printf ("ERROR: Point not within canvas.\n");
      break;

    case ERR_LINEMAX:
      printf ("ERROR: Invalid Line Number.\n");
      break;

    case ERR_LINE:
      printf ("ERROR: Endpoint(s) of line not within canvas.\n");
      break;

    case ERR_BOXMAX:
      printf ("ERROR: Invalid Box Number.\n");
      break;

    case ERR_BOX:
      printf ("ERROR: Top left corner and/or bottom right corner of box not within canvas.\n");
      break;

    case ERR_BOXCORNER:
      printf ("ERROR: Invalid corner(s).\n");
      break;

    case ERR_CIRCLEMAX:
      printf ("ERROR: Invalid Circle Number.\n");
      break;

    case ERR_RADIUSMAX:
      printf ("ERROR: Invalid Circle Radius.\n");
      break;

    case ERR_CENTER:
      printf ("ERROR: Center of Circle not within canvas.\n");
      break;

    case ERR_MOVPT:
      printf ("ERROR: Center of Shape not within canvas.\n");
      break;

    case ERR_MOVSHAPE:
      printf ("ERROR: Some points of shape not within canvas.\n");
      break;
    }
}

/*delete_point deletes the point by deactivating the occupied flag*/

void delete_point (char cmd [][CHAR_MAX], pts_ptr ps)
{
  char *number;
  int pnum;

  number = cmd[1];
  number++;
  pnum = atoi(number) - 1;

  if ((pnum > -1) && (pnum < POINT_MAX))
    {
      if (ps->pt[pnum].occupied) ps->pt[pnum].occupied = 0;
    }
}

/*delete_line deletes the line by deactivating the occupied flag*/

void delete_line (char cmd [][CHAR_MAX], ln_ptr ls)
{
  char *number;
  int lnum;

  number = cmd[1];
  number++;
  lnum = atoi(number) - 1;

  if ((lnum > -1) && (lnum < LINE_MAX))
    {
      if (ls->lines[lnum].occupied) ls->lines[lnum].occupied = 0;
    }
}

/*delete_box deletes the box by deactivating the occupied flag*/

void delete_box (char cmd [][CHAR_MAX], bx_ptr bs)
{
  char *number;
  int bnum;

  number = cmd[1];
  number++;
  bnum = atoi(number) - 1;

  if ((bnum > -1) && (bnum < BOX_MAX))
    {
      if (bs->boxes[bnum].occupied) bs->boxes[bnum].occupied = 0;
    }
}

/*delete_circle deletes the circle by deactivating the occupied flag*/

void delete_circle (char cmd [][CHAR_MAX], cir_ptr cs)
{
  char *number;
  int cnum;

  number = cmd[1];
  number++;
  cnum = atoi(number) - 1;

  if ((cnum > -1) && (cnum < CIRCLE_MAX))
    {
      if (cs->circles[cnum].occupied) cs->circles[cnum].occupied = 0;
    }
}

/*move_point moves point by changing the coordinates of the point to the */
/*coordinates found in the command*/

int move_point (char cmd[][CHAR_MAX], pts_ptr ps)
{
  char *number;
  int pnum, colnum, x, y;

  number = cmd[1];
  number++;
  pnum = atoi(number) - 1;

  if ((pnum > -1) && (pnum < POINT_MAX))
    {
      if (ps->pt[pnum].occupied)
	{
	  x = atoi(cmd[2]);
	  y = atoi(cmd[3]);
	  if ((x < 1) || (y < 1) || (x > PIXEL_MAX) || (y > PIXEL_MAX))
	    {
	      return ERR_MOVPT;
	    }
	  else {
	    ps->pt[pnum].x = x;
	    ps->pt[pnum].y = y;
	    colnum = atoi(cmd[4]);
	    if ((colnum < 0) || (colnum > 4)) ps->pt[pnum].color = 1;
	    else ps->pt[pnum].color = colnum;
	  }
	}
    } 
  return NO_ERROR;
} 

/*move_line moves a line by computing the difference of the center of the line and*/
/*the point indicated by the command and by adding this difference to the */
/*coordinate information of the line*/

int move_line (char cmd[][CHAR_MAX], ln_ptr ls)
{
  char *number;
  int lnum, colnum, x1, x2, y1, y2, diffx, diffy, cenx, ceny, ncenx, nceny;

  number = cmd[1];
  number++;
  lnum = atoi(number) - 1;

  if ((lnum > -1) && (lnum < LINE_MAX))
  {
     if (ls->lines[lnum].occupied)
     {
        compute_midpt(&cenx, &ceny, ls->lines[lnum].line1x, ls->lines[lnum].line1y, ls->lines[lnum].line2x, ls->lines[lnum].line2y);
        ncenx = atoi(cmd[2]);
        nceny = atoi(cmd[3]);

        if ((ncenx < 1) || (nceny < 1) || (ncenx > PIXEL_MAX) || (nceny > PIXEL_MAX))
        {
           return ERR_MOVPT;
        }
        else {
          diffx = ncenx - cenx;
          diffy = nceny - ceny;
        
          x1 = ls->lines[lnum].line1x + diffx;
          x2 = ls->lines[lnum].line2x + diffx;
          y1 = ls->lines[lnum].line1y + diffy;
          y2 = ls->lines[lnum].line2y + diffy;
 
          if ((x1 < 1) || (y1 < 1) || (x2 < 1) || (y2 < 1) || (x1 > PIXEL_MAX) || (y1 > PIXEL_MAX) || (x2 > PIXEL_MAX) || (y2 > PIXEL_MAX))
          {
             return ERR_MOVSHAPE;
          }
          else {
            ls->lines[lnum].line1x = x1;
            ls->lines[lnum].line2x = x2;
            ls->lines[lnum].line1y = y1;
            ls->lines[lnum].line2y = y2;
            colnum = atoi(cmd[4]);
            if ((colnum < 0) || (colnum > 4)) ls->lines[lnum].color = 1;
            else ls->lines[lnum].color = colnum;
          }
        }
     }
  }
  return NO_ERROR;
}

/*move_box moves a line by computing the difference of the center of the box and*/
/*the point indicated by the command and by adding this difference to the */
/*coordinate information of the box*/

int move_box (char cmd[][CHAR_MAX], bx_ptr bs)
{
  char *number;
  int bnum, colnum, x1, x2, y1, y2, diffx, diffy, cenx, ceny, ncenx, nceny;

  number = cmd[1];
  number++;
  bnum = atoi(number) - 1;

  if ((bnum > -1) && (bnum < BOX_MAX))
  {
     if (bs->boxes[bnum].occupied)
     {
        compute_midpt(&cenx, &ceny, bs->boxes[bnum].top_leftx, bs->boxes[bnum].top_lefty, bs->boxes[bnum].bot_rightx, bs->boxes[bnum].bot_righty);
        ncenx = atoi(cmd[2]);
        nceny = atoi(cmd[3]);

        if ((ncenx < 1) || (nceny < 1) || (ncenx > PIXEL_MAX) || (nceny > PIXEL_MAX))
        {
           return ERR_MOVPT;
        }
        else {
          diffx = ncenx - cenx;
          diffy = nceny - ceny;
        
          x1 = bs->boxes[bnum].top_leftx + diffx;
          x2 = bs->boxes[bnum].bot_rightx + diffx;
          y1 = bs->boxes[bnum].top_lefty + diffy;
          y2 = bs->boxes[bnum].bot_righty + diffy;
 
          if ((x1 < 1) || (y1 < 1) || (x2 < 1) || (y2 < 1) || (x1 > PIXEL_MAX) || (y1 > PIXEL_MAX) || (x2 > PIXEL_MAX) || (y2 > PIXEL_MAX))
          {
             return ERR_MOVSHAPE;
          }
          else {
            bs->boxes[bnum].top_leftx = x1;
            bs->boxes[bnum].bot_rightx = x2;
            bs->boxes[bnum].top_lefty = y1;
            bs->boxes[bnum].bot_righty = y2;

            colnum = atoi(cmd[4]);
            if ((colnum < 0) || (colnum > 4)) bs->boxes[bnum].color = 1;
            else bs->boxes[bnum].color = colnum;
          }
        }
     }
  }
  return NO_ERROR;
}

/*move_circle moves point by changing the center of the point to the */
/*coordinates found in the command*/

int move_circle (char cmd[][CHAR_MAX], cir_ptr cs)
{
  char *number;
  int cnum, colnum, cenx, ceny;

  number = cmd[1];
  number++;
  cnum = atoi(number) - 1;

  if ((cnum > -1) && (cnum < CIRCLE_MAX))
    {
      if (cs->circles[cnum].occupied)
	{
	  cenx = atoi(cmd[2]);
	  ceny = atoi(cmd[3]);
	  if ((cenx < 1) || (ceny < 1) || (cenx > PIXEL_MAX) || (ceny > PIXEL_MAX))
	    {
	      return ERR_MOVPT;
	    }
	  else {
	    cs->circles[cnum].centerx = cenx;
	    cs->circles[cnum].centery = ceny;
	    colnum = atoi(cmd[4]);
	    if ((colnum < 0) || (colnum > 4)) cs->circles[cnum].color = 1;
	    else cs->circles[cnum].color = colnum;
	  }
	}
    } 
  return NO_ERROR;
}

/*save_work saves the objects created by creating and then writing info to the*/
/*bitmap file*/

int save_work (char cmd[][CHAR_MAX], pts_ptr ps, ln_ptr ls, bx_ptr bs, cir_ptr cs, graph_ptr g)
{
  FILE *fp;
  int loop, loop2, count, count2, x, y;
  char pixel, display[200][200];
  
  fp = fopen (cmd[1], "wb");

  if (fp == NULL) return ERR_CREATEFILE;

  if (BMPheader(fp)) return ERR_HEADER;

  for (loop = 0; loop < 200; loop++)
    {
      for (loop2 = 0; loop2 < 200; loop2++)
	{
	  display[loop][loop2] = 0;
	}
    }

  create_point (display, ps);
  create_line (display, ls);
  create_box (display, bs);
  create_circle (display, cs);
  create_graph (display, g);


  for (loop = 0; loop < 200; loop++)
    {
      for (loop2 = 0; loop2 < 200; loop2++)
	{
	  pixel = display[loop][loop2];
	  fwrite (&pixel, 1, 1, fp);
	}
    }

  fclose (fp);

  return NO_ERROR;
}

/*compute_midpt computes the mid point of two points*/

void compute_midpt (int *centerx, int *centery, int x1, int y1, int x2, int y2)
{
  double diffx, diffy, cenx, ceny;

  if (x1 < x2) diffx = (double) ((x2 - x1)/2.0);
  else diffx = (double) ((x1 - x2)/2.0);

  if (y1 < y2) diffy = (double) ((y2 - y1)/2.0);
  else diffy = (double) ((y1 - y2)/2.0);

  if (x1 < x2) cenx = (((double)(x1)) + diffx);
  else cenx = (((double)(x2)) + diffx);

  if (y1 < y2) ceny = (((double)(y1)) + diffy);
  else ceny = (((double)(y2)) + diffy);

  *centerx = round_off(cenx);
  *centery = round_off(ceny);
}

/*round_off rounds off a decimal to an integer*/

int round_off (double entry)
{
  char buf[128];
  char* marker;
  int flag, result;

  sprintf(buf, "%.1f", entry);
  /* locate the decimal point */
  marker = strchr(buf, '.');
  if (marker == NULL)
    {
      printf("Error in roundoff function!\n");
      exit(1);
    }
  
  if (entry >= 0)
    {
      if ((*(marker+1)-'0') > ('4' - '0'))
	{
	  flag = 1;  /*means we need to add 1*/
	}
      else
	{
	  flag=0;
	}
      *(marker + 1) = '\0';
      sscanf(buf, "%d", &result);
      if (flag)
	{
	  result += 1;
	}
    }

  else 
    {
      if ((*(marker+1)-'0') > ('4' - '0'))
	{
	  flag = 1;  /*means we need to add -1*/
	}
      else
	{
	  flag=0;
	}
      *(marker + 1) = '\0';
      sscanf(buf, "%d", &result);
      if (flag)
	{
	  result += -1;
	}
    }

  return result;
}

/*compute_slope computes the slope of a line*/

double compute_slope (int x1, int y1, int x2, int y2)
{
  double res, xc, yc;

  xc = (double) (x1 - x2);
  yc = (double) (y1 - y2);
  res = yc / xc;
  return res;
}

/*create_point creates point by placing values to char array*/

void create_point (char display [][PIXEL_MAX], pts_ptr ps)
{
  int index;

  for (index = 0; index < POINT_MAX; index++)
    {
      if (ps->pt[index].occupied) display [ps->pt[index].y-1][ps->pt[index].x-1] = ps->pt[index].color;
    }
}

/*create_line creates point by placing values to char array*/

void create_line (char display [][PIXEL_MAX], ln_ptr ls)
{
  int x1, x2, y1, y2, xa, ya, xb, yb, count, x, y, diffx, diffy, index;
  double countd, slope, xd, yd, part;

  for (index = 0; index < LINE_MAX; index++)
    {
     if (ls->lines[index].occupied)
       {
	 xa = ls->lines[index].line1x-1;
	 ya = ls->lines[index].line1y-1;
	 xb = ls->lines[index].line2x-1;
	 yb = ls->lines[index].line2y-1;

	 /*Vertical Line*/
	 if (xa == xb)
	   {
	     if (ya > yb)
	       {
		 x1 = xb;
		 x2 = xa;
		 y1 = yb;
		 y2 = ya;
	       }
	     else {
      	
	       x1 = xa;
	       x2 = xb;
	       y1 = ya;
	       y2 = yb;
	     }
          
	     for (count = y1; count <= y2; count++) display[count][x1] = ls->lines[index].color;
	   }
	 else {
         
	   /*Horizontal Line*/
           if (ya == yb)
	     {
	       if (xa > xb)
		 {
	           x1 = xb;
	           x2 = xa;
 	           y1 = yb;
   	           y2 = ya;
		 }
	       else {
	        
		 x1 = xa;
		 x2 = xb;
		 y1 = ya;
		 y2 = yb;
	       }
          
	       for (count = x1; count <= x2; count++) display[y1][count] = ls->lines[index].color;
	     }
           else {
	     
	     /*Line with steep slope*/
	     slope = compute_slope(xa, ya, xb, yb);
	     if ((slope > 1) || (slope < -1))
	       {
		 if (xa > xb)
		   {
		     x1 = xb;
		     x2 = xa;
		     y1 = yb;
		     y2 = ya;
		   }
		 else {
		   x1 = xa;
		   x2 = xb;
		   y1 = ya;
		   y2 = yb;
		 }
		 diffx = x2 - x1;
		 diffy = compute_diff(y1, y2) + 1;
		 part = ((double) diffx) / ((double) diffy);
	      
		 for (count = 0; count <= diffy; count++)
		   {
		     countd = (double) count;
		     xd = (countd * part) + x1;
		     yd = (slope * (xd - x1)) + y1;
		     x = round_off(xd);
		     y = round_off(yd);
		     display[y][x] = ls->lines[index].color;
		   }
	       }
	     else {
	    
	       /*Line with flat or regular slope*/
	       if ((slope <= 1) && (slope >= -1))
		 {
		   if (xa > xb)
		     {
		       x1 = xb;
		       x2 = xa;
		       y1 = yb;
		       y2 = ya;
		     }
		   else {
		     x1 = xa;
		     x2 = xb;
		     y1 = ya;
		     y2 = yb;
		   }
		   for (count = x1; count <= x2; count++)
		     {
		       xd = (double) count;
		       yd = (slope * (xd - x1)) + y1;
		       x = round_off(xd);
		       y = round_off(yd);
		       display[y][x] = ls->lines[index].color;
		     }
		 }
	     }
	   }
	 }
       }
    }
}

/*create_box creates box by placing values to char array*/

void create_box (char display [][PIXEL_MAX], bx_ptr bs)
{
  int index, count, count2;

  for (index = 0; index < BOX_MAX; index++)
    {
      if (bs->boxes[index].occupied)
	{
	  for (count = (bs->boxes[index].bot_righty - 1); count < bs->boxes[index].top_lefty; count++)
	    {
	      for (count2 = (bs->boxes[index].top_leftx - 1); count2 < bs->boxes[index].bot_rightx;count2++) 
		{
		  display[count][count2] = bs->boxes[index].color;
		}
	    } 
	}
    }
}

/*create_circle creates circle by placing values to char array*/

void create_circle (char display [][200], cir_ptr cs)
{
  int index, count, count2, x, y, cenx, ceny, ptx, pty;
  double rad, par;

  for (index = 0; index < CIRCLE_MAX; index++)
    {
      if (cs->circles[index].occupied)
	{
	  cenx = cs->circles[index].centerx - 1;
	  ceny = cs->circles[index].centery - 1;
	  for (count = 0; count < 360; count++)
	    {
	      rad = acos(-1);
	      par = (double) (count);
	      par = par / 180.0;
	      rad = rad * par;
	      for (count2 = 0; count2 <= cs->circles[index].radius; count2++)
		{
		  x =  round_off(count2 * (cos(rad)));
		  y =  round_off(count2 * (sin(rad)));
		  ptx = cenx + x;
		  pty = ceny + y;
		  if ((ptx >= 0) && (pty >= 0) && (ptx < PIXEL_MAX) && (pty < PIXEL_MAX))
		    {
		      if ((count2 != cs->circles[index].radius) && (ptx != 0) && (ptx != 199) && (pty != 0) && (pty != 199))
			{ 
			  display[pty][ptx+1] = cs->circles[index].color;
			  display[pty][ptx-1] = cs->circles[index].color;
			  display[pty+1][ptx] = cs->circles[index].color;
			  display[pty-1][ptx] = cs->circles[index].color;
			}
		      display[pty][ptx] = cs->circles[index].color;
		    }
		}
	    }
	}
    }
}

/*create_graph creates graph by placing values to char array*/

void create_graph (char display[][PIXEL_MAX], graph_ptr g)
{
  int count, count2, count3, count4, y;
  float mark, diff, half, min, max;

  if (g->occupied)
    {
      /*create the grid lines*/
      for (count4 = 0; count4 < PIXEL_MAX; count4++)
	{
	  display[99][count4] = 1;
	  display[count4][99] = 1;
	}
      for (count = 0; count < PIXEL_MAX; count++)
	{
	  mark = g->data[count];
	  mark = (mark + 1) * 100;
	  y = round_off((double)mark);
	  if (y > 0)
	    {
	      display[y-1][count] = g->color;
	    }
	  else display[0][count] = g->color;
	}
    }
}

/*compute_diff computes the difference (absolute value) of 2 numbers*/

int compute_diff (int a, int b)
{
  if (a > b) return (a-b);
  else return (b-a);
}

/*BMPheader function*/

int BMPheader (FILE *fp)
{
  unsigned short int sDummy;
  unsigned long int lDummy;
  unsigned int loop;

  /*Check if sizes are correct*/

  if (sizeof(sDummy) != 2) printf("Dsfds\n");
  if (sizeof(lDummy) != 4) printf("dsfadsf\n");

  fputc ('B', fp);              /* BITMAP ID */
  fputc('M', fp);

  lDummy = 41078;               /* File Size */
  fwrite (&lDummy, 4, 1, fp);

  lDummy = 0;                   /* Reserved */
  fwrite (&lDummy, 4, 1, fp);

  lDummy = 1078;                /* Bitmap Data Offset */
  fwrite (&lDummy, 4, 1, fp);

  lDummy = 0x28;                /* Bitmap Header Size */
  fwrite (&lDummy, 4, 1, fp);

  lDummy = 200;                 /* Horizontal Width */
  fwrite (&lDummy, 4, 1, fp);

  lDummy = 200;                 /* Veritcal Height */
  fwrite (&lDummy, 4, 1, fp);

  sDummy = 1;                   /* Number of Planes */
  fwrite (&sDummy, 2, 1, fp);

  sDummy = 8;                   /* Bits Per Pixel */
  fwrite (&sDummy, 2, 1, fp);

  lDummy = 0;                   /* Compression */
  fwrite (&lDummy, 4, 1, fp);

  lDummy = 40000;               /* Bitmap Data Size */
  fwrite (&lDummy, 4, 1, fp);

  lDummy = 0;                   /* Horizontal Resolution */
  fwrite (&lDummy, 4, 1, fp);

  lDummy = 0;                   /* Vertical Resolution */
  fwrite (&lDummy, 4, 1, fp);

  lDummy = 256;                 /* Number of Colors */
  fwrite (&lDummy, 4, 1, fp);

  lDummy = 0;                   /* Number of Important Colors */
  fwrite (&lDummy, 4, 1, fp);

  /* Palette */

  lDummy = 0x00FFFFFF;          /* WHITE */
  fwrite (&lDummy, 4, 1, fp);

  lDummy = 0x00000000;          /* BLACK */
  fwrite (&lDummy, 4, 1, fp);

  lDummy = 0x00FF0000;          /* RED */
  fwrite (&lDummy, 4, 1, fp);

  lDummy = 0x0000FF00;          /* GREEN */
  fwrite (&lDummy, 4, 1, fp);

  lDummy = 0x000000FF;          /* BLUE */
  fwrite (&lDummy, 4, 1, fp);

  for (loop = 0; loop < 251; loop++)
    {
      lDummy = 0;
      fwrite (&lDummy, 4, 1, fp);
    }

  return 0;
}
