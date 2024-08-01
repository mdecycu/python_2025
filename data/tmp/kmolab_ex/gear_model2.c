/******************************************************************************
 *                                                                            *
 * PROGRAM DESCRIPTION -                                                      *
 *   The following example requires a blank, open part. The code creates a    *
 *   spur gear																  *
 * gear_model2.c															  *
 ******************************************************************************/

#include <stdio.h>
#include <math.h>
#include <uf.h>
#include <uf_defs.h>
#include <uf_curve.h>
#include <uf_modl.h>
#include <uf_disp.h>
#include <uf_ui.h>
#include <uf_csys.h>
#include <uf_obj.h>
#include <uf_mtx.h>
#include <uf_trns.h>
#include <uf_view.h>

#define POLES 6
#define WEIGHT 1.0
#define ORDER 6

#define UF_CALL(X) (report( __FILE__, __LINE__, #X, (X)))

static int report( char *file, int line, char *call, int irc)
{
  if (irc)
  {
     char    messg[133];
     printf("%s, line %d:  %s\n", file, line, call);
     (UF_get_fail_message(irc, messg)) ?
       printf("    returned a %d\n", irc) :
       printf("    returned error %d:  %s\n", irc, messg);
  }
  return(irc);
}


static logical prompt_for_three_numbers(char *prompt, char *item1, char *item2, char *item3,
										double *number1, double *number2, double *number3)
{
    int
        irc,
        resp;
    char
        menu[3][16];
    double
        da[3];
	
    strncpy(&menu[0][0], item1, 15);
    strncpy(&menu[1][0], item2, 15);
	strncpy(&menu[2][0], item3, 15);

	
    da[0] = *number1;
    da[1] = *number2;
	da[2] = *number3;
	
    resp = uc1609(prompt, menu, 3, da, &irc);
    if (resp == 3 || resp == 4)
    {
        *number1 = da[0];
        *number2 = da[1];
		*number3 = da[2];

        return TRUE;
    }
    else return FALSE;
	
}


static void do_ugopen_api(void)
{
	int 	i,j, k_fixup, p_fixup, trim_opts[3], arc_opts[3];
	int 	create_flag=1, style1, method, method1 = 0, flag1;
	int		resp, int_teeth_no, num_instances;

	tag_t	lh_spline, rh_spline, lh_base_spline, rh_base_spline, temp_tag, od_arc_tag, wcs_tag,
			curve_obj[3], lh_fillet, rh_fillet, root_arc_tag, cyl_tag, instances, tooth_gap_tag,
			teeth_grp_tag, teeth_tag[240];

	double	knotseq[12]={0.0,0.0,0.0,0.0,0.0,0.0,1.0,1.0,1.0,1.0,1.0,1.0};
	double	pole_array[POLES*4], poles[6][3], poles1[6][3], poles2[6][3];
	double	base_rad, root_rad, rad2 , theta1, theta2, theta3, delta_theta,
			theta_p, rot_angle, angle1;
	double	t_mat[6][6], l_mat[6][6], u_mat[6][6], b_mat[6][2], y_mat[6][2];
	double	point1[3], point2[3], point3[3], tangent1[3];
	double	pitch_dia=12.0, pressure_angle=20.0, dia_pitch, addendum, pitch_rad, undercut, 
			deddendum, sub1, sub2, sub2sq, sub1sq, x_pitch, teeth_no=48.0, clearance;
	double	vec_x[3] = {1.0, 0.0, 0.0}, zc_dir[3] = {0.0, 0.0, 1.0};
	double	start_angle, end_angle, junk3[3], c_point[3];
	double	magnitude, magnitude1, magnitude2, l1, l2, l_new, parm1;
	double	val1, val2, max_fillet, mtx[9];
	double	delta_angle, length1;
	double	origin[3] ={0.0,0.0,0.0};
	double	tooth_length=2.0, gear_od;

	char	options[6][38]={"Full-depth, 14.5 deg pressure angle",
							"Full-depth, 20 deg pressure angle",
							"Stub, 20 deg pressure angle",
							"Coarse-pitch 20 deg pressure angle",
							"Coarse-pitch 25 deg pressure angle",
							"Fine-pitch 20 deg pressure angle"};
	char	tooth_length_st[16], gear_od_st[16], teeth_no_st[16], angle_st[16];
	char *taper_angle = "0.0";
	char *limit1[2] = {"        ", "        "};
	char	start_limit_st[8], end_limit_st[8];

	
	UF_CURVE_arc_t arc_coords;
	UF_CURVE_line_t ln1;
	uf_list_p_t loop_list, features, instances_feature_list;

	for(i=0;i<6;i++)
	{
	  for(j=0;j<6;j++)
	  {
		  t_mat[i][j]=0.0;
		  l_mat[i][j]=0.0;
		  u_mat[i][j]=0.0;
	  }
	}
	t_mat[0][0]=1.0;
	t_mat[5][5]=1.0;
	t_mat[1][0]=0.32768;
	t_mat[4][5]=t_mat[1][0];
	t_mat[1][1]=0.4096;
	t_mat[4][4]=t_mat[1][1];
	t_mat[1][2]=0.2048;
	t_mat[4][3]=t_mat[1][2];
	t_mat[1][3]=0.0512;
	t_mat[4][2]=t_mat[1][3];
	t_mat[1][4]=0.0064;
	t_mat[4][1]=t_mat[1][4];
	t_mat[1][5]=0.00032;
	t_mat[4][0]=t_mat[1][5];
	t_mat[2][0]=0.07776;
	t_mat[3][5]=t_mat[2][0];
	t_mat[2][1]=0.2592;
	t_mat[3][4]=t_mat[2][1];
	t_mat[2][2]=0.3456;
	t_mat[3][3]=t_mat[2][2];
	t_mat[2][3]=0.2304;
	t_mat[3][2]=t_mat[2][3];
	t_mat[2][4]=0.0768;
	t_mat[3][1]=t_mat[2][4];
	t_mat[2][5]=0.01024;
	t_mat[3][0]=t_mat[2][5];  

	for(i=0;i<6;i++)
	{
		l_mat[i][i]=1.0;
	}
	u_mat[0][0]=1.0;
	u_mat[5][5]=1.0;  
	for(i=1;i<6;i++)
	{
		l_mat[i][0]=t_mat[i][0];
		u_mat[1][i]=t_mat[1][i];
	}
	l_mat[2][1]=t_mat[2][1]/t_mat[1][1];
	l_mat[3][1]=t_mat[3][1]/t_mat[1][1];
	l_mat[4][1]=t_mat[4][1]/t_mat[1][1];
	u_mat[2][2]=t_mat[2][2]-l_mat[2][1]*u_mat[1][2];
	u_mat[2][3]=t_mat[2][3]-l_mat[2][1]*u_mat[1][3];
	u_mat[2][4]=t_mat[2][4]-l_mat[2][1]*u_mat[1][4];
	u_mat[2][5]=t_mat[2][5]-l_mat[2][1]*u_mat[1][5];
	l_mat[3][2]=(t_mat[3][2]-l_mat[3][1]*u_mat[1][2])/u_mat[2][2];
	l_mat[4][2]=(t_mat[4][2]-l_mat[4][1]*u_mat[1][2])/u_mat[2][2];
	u_mat[3][3]=t_mat[3][3]-l_mat[3][1]*u_mat[1][3]-l_mat[3][2]*u_mat[2][3];
	u_mat[3][4]=t_mat[3][4]-l_mat[3][1]*u_mat[1][4]-l_mat[3][2]*u_mat[2][4];
	u_mat[3][5]=t_mat[3][5]-l_mat[3][1]*u_mat[1][5]-l_mat[3][2]*u_mat[2][5];
	l_mat[4][3]=(t_mat[4][3]-l_mat[4][1]*u_mat[1][3]-l_mat[4][2]*u_mat[2][3]-l_mat[4][3]*u_mat[3][4])/u_mat[3][3];
	u_mat[4][4]=t_mat[4][4]-l_mat[4][1]*u_mat[1][4]-l_mat[4][2]*u_mat[2][4]-l_mat[4][3]*u_mat[3][4];
	u_mat[4][5]=t_mat[4][5]-l_mat[4][1]*u_mat[1][5]-l_mat[4][2]*u_mat[2][5]-l_mat[4][3]*u_mat[3][5];
	
	restart: ;

	resp = uc1603 ("Standard Imperial Involute Gear Systems to ANSI & AGMA", 0, options, 6 );

	if (resp == 2) goto ending1;

	prompt_for_three_numbers("Gear Data", "Pitch Diameter", "No of Teeth", "Tooth length",
			&pitch_dia, &teeth_no, &tooth_length);
	style1 = resp - 4;
	pitch_rad = pitch_dia/2.0;
	switch (style1)
	{
	case 1:
		// Full-depth, 14.5 deg press angle
		pressure_angle = 14.5;
		base_rad = pitch_rad*cos(pressure_angle*DEGRA);
		dia_pitch = teeth_no/pitch_dia;
		addendum = 1.0/dia_pitch;
		deddendum = 1.157/dia_pitch;
		clearance = 0.157/dia_pitch;
		max_fillet = 1.5*clearance;
		break;
	case 2:
		// Full-depth, 20 deg press angle
		pressure_angle = 20.0;
		base_rad = pitch_rad*cos(pressure_angle*DEGRA);
		dia_pitch = teeth_no/pitch_dia;
		addendum = 1.0/dia_pitch;
		deddendum = 1.157/dia_pitch;
		clearance = 0.157/dia_pitch;
		max_fillet = 1.5*clearance;
		break;
	case 3:
		// Stub, 20 deg pressure angle
		pressure_angle = 20.0;
		base_rad = pitch_rad*cos(pressure_angle*DEGRA);
		dia_pitch = teeth_no/pitch_dia;
		addendum = 0.8/dia_pitch;
		deddendum = 1.0/dia_pitch;
		clearance = 0.2/dia_pitch;
		max_fillet = 1.5*clearance;
		break;
	case 4:
		// Coarse-pitch 20 deg press angle
		pressure_angle = 20.0;
		base_rad = pitch_rad*cos(pressure_angle*DEGRA);
		dia_pitch = teeth_no/pitch_dia;
		addendum = 1.0/dia_pitch;
		deddendum = 1.25/dia_pitch;
		clearance = 0.25/dia_pitch;
		max_fillet = 0.3/dia_pitch;
		break;
	case 5:
		// Coarse-pitch 25 deg press angle
		pressure_angle = 25.0;
		base_rad = pitch_rad*cos(pressure_angle*DEGRA);
		dia_pitch = teeth_no/pitch_dia;
		addendum = 1.0/dia_pitch;
		deddendum = 1.25/dia_pitch;
		clearance = 0.25/dia_pitch;
		max_fillet = 0.3/dia_pitch;
		break;
	case 6:
		// Fine-pitch 20 deg press angle
		pressure_angle = 20.0;
		base_rad = pitch_rad*cos(pressure_angle*DEGRA);
		dia_pitch = teeth_no/pitch_dia;
		addendum = 1.0/dia_pitch;
		deddendum = 0.002+1.2/dia_pitch;
		clearance = 0.002+0.2/dia_pitch;
		max_fillet = 0.3/dia_pitch;
		break;
	}
	
	root_rad = pitch_rad - deddendum;
	gear_od = (pitch_rad + addendum)*2.0;

//	Determine od involute angle
	
	sub1 = (pitch_rad+addendum)/base_rad;
	theta2 = sqrt(sub1*sub1-1.0);

	sprintf(gear_od_st,"%f",gear_od);
	sprintf(tooth_length_st,"TOOTH_LENGTH1=%f",tooth_length);

	UF_MODL_create_cyl1 (UF_NULLSIGN, origin,tooth_length_st, gear_od_st, 
			zc_dir, &cyl_tag);

	sprintf(start_limit_st,"0.0");
	sprintf(end_limit_st,"TOOTH_LENGTH1");
	strcpy(&limit1[0][0],start_limit_st);
	strcpy(&limit1[1][0],end_limit_st);

	if (root_rad < base_rad) method = 0;
	if (root_rad > base_rad) method = 1;

	switch (method)
	{
	case 0:
		//	Create normal section splines
		theta1 = 0.0;
		theta3 = 0.0;
		delta_theta = (theta2-theta1)/5.0; 
		for (i=0;i<6;i++)
		{
			b_mat[i][0] = base_rad*sin(theta3)- base_rad*theta3*cos(theta3);
			b_mat[i][1] = base_rad*cos(theta3)+ base_rad*theta3*sin(theta3);
			theta3 = theta3 + delta_theta;
		}
		y_mat[0][0] = b_mat[0][0]/l_mat[0][0];
		y_mat[0][1] = b_mat[0][1]/l_mat[0][0];
		y_mat[1][0]=(b_mat[1][0]-l_mat[1][0]*y_mat[0][0])/l_mat[1][1];
		y_mat[1][1]=(b_mat[1][1]-l_mat[1][0]*y_mat[0][1])/l_mat[1][1];
		y_mat[2][0]=(b_mat[2][0]-l_mat[2][0]*y_mat[0][0]-l_mat[2][1]*y_mat[1][0])/l_mat[2][2];
		y_mat[2][1]=(b_mat[2][1]-l_mat[2][0]*y_mat[0][1]-l_mat[2][1]*y_mat[1][1])/l_mat[2][2];
		y_mat[3][0]=(b_mat[3][0]-l_mat[3][0]*y_mat[0][0]-l_mat[3][1]*y_mat[1][0]-l_mat[3][2]*y_mat[2][0])/l_mat[3][3];
		y_mat[3][1]=(b_mat[3][1]-l_mat[3][0]*y_mat[0][1]-l_mat[3][1]*y_mat[1][1]-l_mat[3][2]*y_mat[2][1])/l_mat[3][3];
		y_mat[4][0]=(b_mat[4][0]-l_mat[4][0]*y_mat[0][0]-l_mat[4][1]*y_mat[1][0]-l_mat[4][2]*y_mat[2][0]-
							l_mat[4][3]*y_mat[3][0])/l_mat[4][4];
		y_mat[4][1]=(b_mat[4][1]-l_mat[4][0]*y_mat[0][1]-l_mat[4][1]*y_mat[1][1]-l_mat[4][2]*y_mat[2][1]-
							l_mat[4][3]*y_mat[3][1])/l_mat[4][4];
		y_mat[5][0]=(b_mat[5][0]-l_mat[5][0]*y_mat[0][0]-l_mat[5][1]*y_mat[1][0]-l_mat[5][2]*y_mat[2][0]-
							l_mat[5][3]*y_mat[3][0]-l_mat[5][4]*y_mat[4][0])/l_mat[5][5];
		y_mat[5][1]=(b_mat[5][1]-l_mat[5][0]*y_mat[0][1]-l_mat[5][1]*y_mat[1][1]-l_mat[5][2]*y_mat[2][1]-
							l_mat[5][3]*y_mat[3][1]-l_mat[5][4])/l_mat[5][5];
		poles[5][0]=y_mat[5][0];
		poles[5][1]=y_mat[5][1];
		poles[4][0]=(y_mat[4][0]-u_mat[4][5]*poles[5][0])/u_mat[4][4];
		poles[4][1]=(y_mat[4][1]-u_mat[4][5]*poles[5][1])/u_mat[4][4];
		poles[3][0]=(y_mat[3][0]-u_mat[3][4]*poles[4][0]-u_mat[3][5]*poles[5][0])/u_mat[3][3];
		poles[3][1]=(y_mat[3][1]-u_mat[3][4]*poles[4][1]-u_mat[3][5]*poles[5][1])/u_mat[3][3];
		poles[2][0]=(y_mat[2][0]-u_mat[2][3]*poles[3][0]-u_mat[2][4]*poles[4][0]-u_mat[2][5]*poles[5][0])/u_mat[2][2];
		poles[2][1]=(y_mat[2][1]-u_mat[2][3]*poles[3][1]-u_mat[2][4]*poles[4][1]-u_mat[2][5]*poles[5][1])/u_mat[2][2];
		poles[1][0]=(y_mat[1][0]-u_mat[1][2]*poles[2][0]-u_mat[1][3]*poles[3][0]-u_mat[1][4]*poles[4][0]-
					u_mat[1][5]*poles[5][0])/u_mat[1][1];
		poles[1][1]=(y_mat[1][1]-u_mat[1][2]*poles[2][1]-u_mat[1][3]*poles[3][1]-u_mat[1][4]*poles[4][1]-
					u_mat[1][5]*poles[5][1])/u_mat[1][1];
		poles[0][0]=y_mat[0][0]-u_mat[0][1]*poles[1][0]-u_mat[0][2]*poles[2][0]-u_mat[0][3]*poles[3][0]-
					u_mat[0][4]*poles[4][0]-u_mat[0][5]*poles[5][0];
		poles[0][1]=y_mat[0][1]-u_mat[0][1]*poles[1][1]-u_mat[0][2]*poles[2][1]-u_mat[0][3]*poles[3][1]-
					u_mat[0][4]*poles[4][1]-u_mat[0][5]*poles[5][1];
		poles[1][0]=0.0;
		poles[0][0]=0.0;
 //  Calculate rotation angle
		rad2 = pitch_dia/2.0;
		sub1 = rad2/base_rad;
		sub1sq = sub1*sub1;
		theta_p = sqrt(sub1sq-1.0);
		x_pitch = base_rad*sin(theta_p)-base_rad*theta_p*cos(theta_p);
		rot_angle = PI/(2.0*teeth_no) - asin(x_pitch/rad2);
		for (i=0;i<6;i++)
		{
			poles1[i][0] = poles[i][0]*cos(-rot_angle)-poles[i][1]*sin(-rot_angle);
			poles1[i][1] = poles[i][0]*sin(-rot_angle)+poles[i][1]*cos(-rot_angle);
			poles1[i][2] = 0.0;
		}  
		for (i=0;i<6;i++)
		{
			poles2[i][0] = -poles1[i][0];
			poles2[i][1] = poles1[i][1];
			poles2[i][2] = 0.0;
		}
		pole_array[0] = poles1[0][0];
		pole_array[1] = poles1[0][1];
		pole_array[2] = 0.0;
		pole_array[3] = 1.0;
		pole_array[4] = poles1[1][0];
		pole_array[5] = poles1[1][1];
		pole_array[6] = 0.0;
		pole_array[7] = 1.0;
		pole_array[8] = poles1[2][0];
		pole_array[9] = poles1[2][1];
		pole_array[10] = 0.0;
		pole_array[11] = 1.0;
		pole_array[12] = poles1[3][0];
		pole_array[13] = poles1[3][1];
		pole_array[14] = 0.0;
		pole_array[15] = 1.0;
		pole_array[16] = poles1[4][0];
		pole_array[17] = poles1[4][1];
		pole_array[18] = 0.0;
		pole_array[19] = 1.0;
		pole_array[20] = poles1[5][0];
		pole_array[21] = poles1[5][1];
		pole_array[22] = 0.0;
		pole_array[23] = 1.0;
		UF_CALL(UF_MODL_create_spline(POLES,ORDER,knotseq,pole_array,
						&lh_spline, &k_fixup,&p_fixup));
		pole_array[0] = poles2[0][0];
		pole_array[1] = poles2[0][1];
		pole_array[2] = 0.0;
		pole_array[3] = 1.0;
		pole_array[4] = poles2[1][0];
		pole_array[5] = poles2[1][1];
		pole_array[6] = 0.0;
		pole_array[7] = 1.0;
		pole_array[8] = poles2[2][0];
		pole_array[9] = poles2[2][1];
		pole_array[10] = 0.0;
		pole_array[11] = 1.0;
		pole_array[12] = poles2[3][0];
		pole_array[13] = poles2[3][1];
		pole_array[14] = 0.0;
		pole_array[15] = 1.0;
		pole_array[16] = poles2[4][0];
		pole_array[17] = poles2[4][1];
		pole_array[18] = 0.0;
		pole_array[19] = 1.0;
		pole_array[20] = poles2[5][0];
		pole_array[21] = poles2[5][1];
		pole_array[22] = 0.0;
		pole_array[23] = 1.0;
		UF_CALL(UF_MODL_create_spline(POLES,ORDER,knotseq,pole_array,
						&rh_spline, &k_fixup,&p_fixup));
		point1[0] = 0.0;
		point1[1] = pitch_dia/2.0 + addendum;
		point1[2] = 0.0;
		UF_CURVE_create_arc_thru_3pts ( create_flag, &poles1[5][0],  point1, &poles2[5][0], &od_arc_tag);
		UF_MODL_ask_curve_props(lh_spline, 0.0, &point1[0], tangent1, junk3, junk3, junk3,junk3);
		UF_VEC3_angle_between(vec_x,&poles1[0][0],zc_dir,&start_angle);
		point2[0] = root_rad*cos(start_angle);
		point2[1] = root_rad*sin(start_angle);
		point2[2] = 0.0;
		ln1.start_point[0] = point2[0];
		ln1.start_point[1] = point2[1];
		ln1.start_point[2] = 0.0;
		ln1.end_point[0] = point1[0];
		ln1.end_point[1] = point1[1];
		ln1.end_point[2] = 0.0;
		UF_CALL(UF_CURVE_create_line(&ln1,&lh_base_spline));
		UF_MODL_ask_curve_props(rh_spline, 0.0, &point1[0], tangent1, junk3, junk3, junk3,junk3);
		UF_VEC3_angle_between(vec_x,&poles2[0][0],zc_dir,&end_angle);
		point3[0] = root_rad*cos(end_angle);
		point3[1] = root_rad*sin(end_angle);
		point3[2] = 0.0;
		ln1.start_point[0] = point3[0];
		ln1.start_point[1] = point3[1];
		ln1.start_point[2] = 0.0;
		ln1.end_point[0] = point1[0];
		ln1.end_point[1] = point1[1];
		ln1.end_point[2] = 0.0;
		UF_CALL(UF_CURVE_create_line(&ln1,&rh_base_spline));
		UF_VEC3_distance(point3, point1, &length1);
		point1[0] = 0.0;
		point1[1] = root_rad;
		point1[2] = 0.0;
		UF_CURVE_create_arc_thru_3pts ( create_flag, point2,  point1, point3, &root_arc_tag);
		if (length1 > max_fillet)
		{
			curve_obj[0] = root_arc_tag;
			curve_obj[1] = lh_base_spline;
			point1[0] = poles1[0][0] - max_fillet;
			point1[1] = poles1[0][1] - max_fillet;
			point1[2] = 0.0;
			trim_opts[0] = TRUE;
			trim_opts[1] = TRUE;
			arc_opts[0] = UF_CURVE_TANGENT_OUTSIDE;
			UF_CURVE_create_fillet (UF_CURVE_2_CURVE, curve_obj, point1, 
				max_fillet, trim_opts, arc_opts, &lh_fillet); 
			curve_obj[0] = rh_base_spline;
			curve_obj[1] = root_arc_tag;
			point1[0] = poles2[0][0] + max_fillet;
			point1[1] = poles2[0][1] - max_fillet;
			point1[2] = 0.0;
			trim_opts[0] = TRUE;
			trim_opts[1] = TRUE;
			arc_opts[1] = UF_CURVE_TANGENT_OUTSIDE;
			UF_CURVE_create_fillet (UF_CURVE_2_CURVE, curve_obj, point1, 
				max_fillet, trim_opts, arc_opts, &rh_fillet);
			UF_CALL(UF_MODL_create_list(&loop_list));
			UF_CALL(UF_MODL_put_list_item(loop_list, od_arc_tag));
			UF_CALL(UF_MODL_put_list_item(loop_list, lh_spline));
			UF_CALL(UF_MODL_put_list_item(loop_list, lh_base_spline));
			UF_CALL(UF_MODL_put_list_item(loop_list, lh_fillet));
			UF_CALL(UF_MODL_put_list_item(loop_list, root_arc_tag));
			UF_CALL(UF_MODL_put_list_item(loop_list, rh_fillet));
			UF_CALL(UF_MODL_put_list_item(loop_list, rh_base_spline));
			UF_CALL(UF_MODL_put_list_item(loop_list, rh_spline));
			UF_MODL_create_extruded1 (loop_list, taper_angle, limit1,
                          origin, zc_dir, UF_NEGATIVE , cyl_tag, &features);
			UF_CALL(UF_MODL_ask_list_item(features, 0 , &tooth_gap_tag));
			UF_MODL_delete_list(&loop_list);
			UF_MODL_delete_list(&features);
			UF_CALL(UF_MODL_create_list(&loop_list));
			UF_CALL(UF_MODL_put_list_item(loop_list, tooth_gap_tag));
			UF_OBJ_set_blank_status (od_arc_tag, UF_OBJ_BLANKED);
			UF_OBJ_set_blank_status (lh_spline, UF_OBJ_BLANKED);
			UF_OBJ_set_blank_status (lh_base_spline, UF_OBJ_BLANKED);
			UF_OBJ_set_blank_status (lh_fillet, UF_OBJ_BLANKED);
			UF_OBJ_set_blank_status (root_arc_tag, UF_OBJ_BLANKED);
			UF_OBJ_set_blank_status (rh_fillet, UF_OBJ_BLANKED);
			UF_OBJ_set_blank_status (rh_base_spline, UF_OBJ_BLANKED);
			UF_OBJ_set_blank_status (rh_spline, UF_OBJ_BLANKED);
		}
		else if (length1 < max_fillet)
		{
			curve_obj[0] = root_arc_tag;
			curve_obj[1] = lh_spline;
			point1[0] = poles1[0][0] - max_fillet;
			point1[1] = poles1[0][1] + max_fillet;
			point1[2] = 0.0;
			trim_opts[0] = TRUE;
			trim_opts[1] = TRUE;
			arc_opts[0] = UF_CURVE_TANGENT_OUTSIDE;
			UF_CURVE_create_fillet (UF_CURVE_2_CURVE, curve_obj, point1, 
					max_fillet, trim_opts, arc_opts, &lh_fillet); 
			curve_obj[0] = rh_spline;
			curve_obj[1] = root_arc_tag;
			point1[0] = poles2[0][0] + max_fillet;
			point1[1] = poles2[0][1] + max_fillet;
			point1[2] = 0.0;
			trim_opts[0] = TRUE;
			trim_opts[1] = TRUE;
			arc_opts[1] = UF_CURVE_TANGENT_OUTSIDE;
			UF_CURVE_create_fillet (UF_CURVE_2_CURVE, curve_obj, point1, 
					max_fillet, trim_opts, arc_opts, &rh_fillet);
			UF_OBJ_delete_object (lh_base_spline);
			UF_OBJ_delete_object (rh_base_spline);
			UF_CALL(UF_MODL_create_list(&loop_list));
			UF_CALL(UF_MODL_put_list_item(loop_list, od_arc_tag));
			UF_CALL(UF_MODL_put_list_item(loop_list, lh_spline));
			UF_CALL(UF_MODL_put_list_item(loop_list, lh_fillet));
			UF_CALL(UF_MODL_put_list_item(loop_list, root_arc_tag));
			UF_CALL(UF_MODL_put_list_item(loop_list, rh_fillet));
			UF_CALL(UF_MODL_put_list_item(loop_list, rh_spline));
			UF_MODL_create_extruded1 (loop_list, taper_angle, limit1,
                          origin, zc_dir, UF_NEGATIVE , cyl_tag, &features);
			UF_CALL(UF_MODL_ask_list_item(features, 0 , &tooth_gap_tag));
			UF_MODL_delete_list(&loop_list);
			UF_MODL_delete_list(&features);
			UF_CALL(UF_MODL_create_list(&loop_list));
			UF_CALL(UF_MODL_put_list_item(loop_list, tooth_gap_tag));
			UF_OBJ_set_blank_status (od_arc_tag, UF_OBJ_BLANKED);
			UF_OBJ_set_blank_status (lh_spline, UF_OBJ_BLANKED);
			UF_OBJ_set_blank_status (lh_fillet, UF_OBJ_BLANKED);
			UF_OBJ_set_blank_status (root_arc_tag, UF_OBJ_BLANKED);
			UF_OBJ_set_blank_status (rh_fillet, UF_OBJ_BLANKED);
			UF_OBJ_set_blank_status (rh_spline, UF_OBJ_BLANKED);
		}
	  break;
	case 1:
		sub2 = (pitch_rad-deddendum)/base_rad;
		sub2sq = sub2*sub2;
		theta1 = sqrt(sub2sq - 1.0);
		theta3 = theta1;
		delta_theta = (theta2-theta1)/5.0; 
		for (i=0;i<6;i++)
		{
			b_mat[i][0] = base_rad*sin(theta3)- base_rad*theta3*cos(theta3);
			b_mat[i][1] = base_rad*cos(theta3)+ base_rad*theta3*sin(theta3);
			theta3 = theta3 + delta_theta;
		} 
		y_mat[0][0] = b_mat[0][0]/l_mat[0][0];
		y_mat[0][1] = b_mat[0][1]/l_mat[0][0];
		y_mat[1][0]=(b_mat[1][0]-l_mat[1][0]*y_mat[0][0])/l_mat[1][1];
		y_mat[1][1]=(b_mat[1][1]-l_mat[1][0]*y_mat[0][1])/l_mat[1][1];
		y_mat[2][0]=(b_mat[2][0]-l_mat[2][0]*y_mat[0][0]-l_mat[2][1]*y_mat[1][0])/l_mat[2][2];
		y_mat[2][1]=(b_mat[2][1]-l_mat[2][0]*y_mat[0][1]-l_mat[2][1]*y_mat[1][1])/l_mat[2][2];
		y_mat[3][0]=(b_mat[3][0]-l_mat[3][0]*y_mat[0][0]-l_mat[3][1]*y_mat[1][0]-l_mat[3][2]*y_mat[2][0])/l_mat[3][3];
		y_mat[3][1]=(b_mat[3][1]-l_mat[3][0]*y_mat[0][1]-l_mat[3][1]*y_mat[1][1]-l_mat[3][2]*y_mat[2][1])/l_mat[3][3];
		y_mat[4][0]=(b_mat[4][0]-l_mat[4][0]*y_mat[0][0]-l_mat[4][1]*y_mat[1][0]-l_mat[4][2]*y_mat[2][0]-
							l_mat[4][3]*y_mat[3][0])/l_mat[4][4];
		y_mat[4][1]=(b_mat[4][1]-l_mat[4][0]*y_mat[0][1]-l_mat[4][1]*y_mat[1][1]-l_mat[4][2]*y_mat[2][1]-
							l_mat[4][3]*y_mat[3][1])/l_mat[4][4];
		y_mat[5][0]=(b_mat[5][0]-l_mat[5][0]*y_mat[0][0]-l_mat[5][1]*y_mat[1][0]-l_mat[5][2]*y_mat[2][0]-
							l_mat[5][3]*y_mat[3][0]-l_mat[5][4]*y_mat[4][0])/l_mat[5][5];
		y_mat[5][1]=(b_mat[5][1]-l_mat[5][0]*y_mat[0][1]-l_mat[5][1]*y_mat[1][1]-l_mat[5][2]*y_mat[2][1]-
							l_mat[5][3]*y_mat[3][1]-l_mat[5][4])/l_mat[5][5];
		poles[5][0]=y_mat[5][0];
		poles[5][1]=y_mat[5][1];
		poles[4][0]=(y_mat[4][0]-u_mat[4][5]*poles[5][0])/u_mat[4][4];
		poles[4][1]=(y_mat[4][1]-u_mat[4][5]*poles[5][1])/u_mat[4][4];
		poles[3][0]=(y_mat[3][0]-u_mat[3][4]*poles[4][0]-u_mat[3][5]*poles[5][0])/u_mat[3][3];
		poles[3][1]=(y_mat[3][1]-u_mat[3][4]*poles[4][1]-u_mat[3][5]*poles[5][1])/u_mat[3][3];
		poles[2][0]=(y_mat[2][0]-u_mat[2][3]*poles[3][0]-u_mat[2][4]*poles[4][0]-u_mat[2][5]*poles[5][0])/u_mat[2][2];
		poles[2][1]=(y_mat[2][1]-u_mat[2][3]*poles[3][1]-u_mat[2][4]*poles[4][1]-u_mat[2][5]*poles[5][1])/u_mat[2][2];
		poles[1][0]=(y_mat[1][0]-u_mat[1][2]*poles[2][0]-u_mat[1][3]*poles[3][0]-u_mat[1][4]*poles[4][0]-
					u_mat[1][5]*poles[5][0])/u_mat[1][1];
		poles[1][1]=(y_mat[1][1]-u_mat[1][2]*poles[2][1]-u_mat[1][3]*poles[3][1]-u_mat[1][4]*poles[4][1]-
					u_mat[1][5]*poles[5][1])/u_mat[1][1];
		poles[0][0]=y_mat[0][0]-u_mat[0][1]*poles[1][0]-u_mat[0][2]*poles[2][0]-u_mat[0][3]*poles[3][0]-
					u_mat[0][4]*poles[4][0]-u_mat[0][5]*poles[5][0];
		poles[0][1]=y_mat[0][1]-u_mat[0][1]*poles[1][1]-u_mat[0][2]*poles[2][1]-u_mat[0][3]*poles[3][1]-
					u_mat[0][4]*poles[4][1]-u_mat[0][5]*poles[5][1];
 //  Calculate rotation angle
		rad2 = pitch_dia/2.0;
		sub1 = rad2/base_rad;
		sub1sq = sub1*sub1;
		theta_p = sqrt(sub1sq-1.0);
		x_pitch = base_rad*sin(theta_p)-base_rad*theta_p*cos(theta_p);
		rot_angle = PI/(2.0*teeth_no) - asin(x_pitch/rad2);
		for (i=0;i<6;i++)
		{
			poles1[i][0] = poles[i][0]*cos(-rot_angle)-poles[i][1]*sin(-rot_angle);
			poles1[i][1] = poles[i][0]*sin(-rot_angle)+poles[i][1]*cos(-rot_angle);
			poles1[i][2] = 0.0;
		}  
		for (i=0;i<6;i++)
		{
			poles2[i][0] = -poles1[i][0];
			poles2[i][1] = poles1[i][1];
			poles2[i][2] = 0.0;
		}
		pole_array[0] = poles1[0][0];
		pole_array[1] = poles1[0][1];
		pole_array[2] = 0.0;
		pole_array[3] = 1.0;
		pole_array[4] = poles1[1][0];
		pole_array[5] = poles1[1][1];
		pole_array[6] = 0.0;
		pole_array[7] = 1.0;
		pole_array[8] = poles1[2][0];
		pole_array[9] = poles1[2][1];
		pole_array[10] = 0.0;
		pole_array[11] = 1.0;
		pole_array[12] = poles1[3][0];
		pole_array[13] = poles1[3][1];
		pole_array[14] = 0.0;
		pole_array[15] = 1.0;
		pole_array[16] = poles1[4][0];
		pole_array[17] = poles1[4][1];
		pole_array[18] = 0.0;
		pole_array[19] = 1.0;
		pole_array[20] = poles1[5][0];
		pole_array[21] = poles1[5][1];
		pole_array[22] = 0.0;
		pole_array[23] = 1.0;
		UF_CALL(UF_MODL_create_spline(POLES,ORDER,knotseq,pole_array,
						&lh_spline, &k_fixup,&p_fixup));
		pole_array[0] = poles2[0][0];
		pole_array[1] = poles2[0][1];
		pole_array[2] = 0.0;
		pole_array[3] = 1.0;
		pole_array[4] = poles2[1][0];
		pole_array[5] = poles2[1][1];
		pole_array[6] = 0.0;
		pole_array[7] = 1.0;
		pole_array[8] = poles2[2][0];
		pole_array[9] = poles2[2][1];
		pole_array[10] = 0.0;
		pole_array[11] = 1.0;
		pole_array[12] = poles2[3][0];
		pole_array[13] = poles2[3][1];
		pole_array[14] = 0.0;
		pole_array[15] = 1.0;
		pole_array[16] = poles2[4][0];
		pole_array[17] = poles2[4][1];
		pole_array[18] = 0.0;
		pole_array[19] = 1.0;
		pole_array[20] = poles2[5][0];
		pole_array[21] = poles2[5][1];
		pole_array[22] = 0.0;
		pole_array[23] = 1.0;
		UF_CALL(UF_MODL_create_spline(POLES,ORDER,knotseq,pole_array,
						&rh_spline, &k_fixup,&p_fixup));
		point1[0] = 0.0;
		point1[1] = pitch_dia/2.0 + addendum;
		point1[2] = 0.0;
		UF_CURVE_create_arc_thru_3pts ( create_flag, &poles1[5][0],  point1, &poles2[5][0], &od_arc_tag);
		point1[0] = 0.0;
		point1[1] = root_rad;
		point1[2] = 0.0;
		UF_CURVE_create_arc_thru_3pts ( create_flag, &poles1[0][0],  point1, &poles2[0][0], &root_arc_tag);
		curve_obj[0] = root_arc_tag;
		curve_obj[1] = lh_spline;
		point1[0] = poles1[0][0];
		point1[1] = poles1[0][1] + max_fillet/2.0;
		point1[2] = 0.0;
		trim_opts[0] = TRUE;
		trim_opts[1] = TRUE;
		arc_opts[0] = UF_CURVE_TANGENT_OUTSIDE;
		UF_CURVE_create_fillet (UF_CURVE_2_CURVE, curve_obj, point1, max_fillet, trim_opts, arc_opts, &lh_fillet);
		curve_obj[0] = rh_spline;
		curve_obj[1] = root_arc_tag;
		point1[0] = poles2[0][0] ;
		point1[1] = poles2[0][1] + max_fillet/2.0;
		point1[2] = 0.0;
		trim_opts[0] = TRUE;
		trim_opts[1] = TRUE;
		arc_opts[1] = UF_CURVE_TANGENT_OUTSIDE;
		UF_CURVE_create_fillet (UF_CURVE_2_CURVE, curve_obj, point1, max_fillet, trim_opts, arc_opts, &rh_fillet);
		UF_CALL(UF_MODL_create_list(&loop_list));
		UF_CALL(UF_MODL_put_list_item(loop_list, od_arc_tag));
		UF_CALL(UF_MODL_put_list_item(loop_list, lh_spline));
		UF_CALL(UF_MODL_put_list_item(loop_list, lh_fillet));
		UF_CALL(UF_MODL_put_list_item(loop_list, root_arc_tag));
		UF_CALL(UF_MODL_put_list_item(loop_list, rh_fillet));
		UF_CALL(UF_MODL_put_list_item(loop_list, rh_spline));
		UF_MODL_create_extruded1 (loop_list, taper_angle, limit1,
                          origin, zc_dir, UF_NEGATIVE , cyl_tag, &features);
		UF_CALL(UF_MODL_ask_list_item(features, 0 , &tooth_gap_tag));
		UF_MODL_delete_list(&loop_list);
		UF_MODL_delete_list(&features);
		UF_CALL(UF_MODL_create_list(&loop_list));
		UF_CALL(UF_MODL_put_list_item(loop_list, tooth_gap_tag));
		UF_OBJ_set_blank_status (od_arc_tag, UF_OBJ_BLANKED);
		UF_OBJ_set_blank_status (lh_spline, UF_OBJ_BLANKED);
		UF_OBJ_set_blank_status (lh_fillet, UF_OBJ_BLANKED);
		UF_OBJ_set_blank_status (root_arc_tag, UF_OBJ_BLANKED);
		UF_OBJ_set_blank_status (rh_fillet, UF_OBJ_BLANKED);
		UF_OBJ_set_blank_status (rh_spline, UF_OBJ_BLANKED);
		break;  
	}
	int_teeth_no = teeth_no;
	sprintf(teeth_no_st,"%d", int_teeth_no);
	angle1 = 360.0/teeth_no;
	sprintf(angle_st,"%f",angle1);
	UF_MODL_create_circular_iset (method1, origin, zc_dir, teeth_no_st, angle_st, loop_list, &instances );

	UF_MODL_ask_instance (instances, &instances_feature_list );

	UF_CALL(UF_MODL_ask_list_count(instances_feature_list,&num_instances));
	for (i=0;i<num_instances;i++)
	{
		UF_CALL(UF_MODL_ask_list_item(instances_feature_list,i,&temp_tag));
		teeth_tag[i] = temp_tag;
	}

	UF_MODL_create_set_of_feature ("Teeth_set", &teeth_tag[0], num_instances, 1, &teeth_grp_tag );

	ending1: ;

	UF_CALL(UF_VIEW_fit_view(NULL_TAG, 1.0));	
}

/*ARGSUSED*/
void ufusr(char *param, int *retcode, int param_len)
{
  if (!UF_CALL(UF_initialize()))
  {
    do_ugopen_api();
    UF_CALL(UF_terminate());
  }
}

int ufusr_ask_unload(void)
{
  return (UF_UNLOAD_IMMEDIATELY);
}
