#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <gphoto2/gphoto2.h>
#include <gphoto2/gphoto2-camera.h>

//#define DO_TETHER_CAPTURE
//#define GET_FILE

double sInterval = 20.;

#define MAX_CAMERAS 2

//==================================================================
// Config Values (just for example, they varies depend on your camera and lens)
//==================================================================

enum {
	FOCUS_NEAR_1 = 0,
	FOCUS_NEAR_2,
	FOCUS_NEAR_3,
	FOCUS_NONE,
	FOCUS_FAR_1,
	FOCUS_FAR_2,
	FOCUS_FAR_3
};

enum {
	APERTURE_2_8 = 0,
	APERTURE_3_2,
	APERTURE_3_5,
	APERTURE_4,
	APERTURE_4_5,
	APERTURE_5,
	APERTURE_5_6,
	APERTURE_6_3,
	APERTURE_7_1,
	APERTURE_8,
	APERTURE_9,
	APERTURE_10,
	APERTURE_11,
	APERTURE_13,
	APERTURE_14,
	APERTURE_16,
	APERTURE_18,
	APERTURE_20,
	APERTURE_22
};

enum {
	SS_30 = 0,
	SS_25,
	SS_20,
	SS_15,
	SS_13,
	SS_10,
	SS_8,
	SS_6,
	SS_5,
	SS_4,
	SS_3_2,
	SS_2_5,
	SS_2,
	SS_1_6,
	SS_1_3,
	SS_1,
	SS_0_8,
	SS_0_6,
	SS_0_5,
	SS_0_4,
	SS_0_3,
	SS_1p4,
	SS_1p5,
	SS_1p6,
	SS_1p8,
	SS_1p10,
	SS_1p13,
	SS_1p15,
	SS_1p20,
	SS_1p25,
	SS_1p30,
	SS_1p40,
	SS_1p50,
	SS_1p60,
	SS_1p80,
	SS_1p100,
	SS_1p125,
	SS_1p160,
	SS_1p200,
	SS_1p250,
	SS_1p320,
	SS_1p400,
	SS_1p500,
	SS_1p640,
	SS_1p800,
	SS_1p1000,
	SS_1p1250,
	SS_1p1600,
	SS_1p2000,
	SS_1p2500,
	SS_1p3200,
	SS_1p4000,
	SS_1p5000,
	SS_1p6400,
	SS_1p8000
};

enum {
	ISO_AUTO = 0,
	ISO_200,
	ISO_250,
	ISO_320,
	ISO_400,
	ISO_500,
	ISO_640,
	ISO_800,
	ISO_1000,
	ISO_1250,
	ISO_1600,
	ISO_2000,
	ISO_2500,
	ISO_3200,
	ISO_4000,
	ISO_5000,
	ISO_6400
};

enum {
	EXP_COMP_M2,
	EXP_COMP_M1_6,
	EXP_COMP_M1_3,
	EXP_COMP_M1_0,
	EXP_COMP_M0_6,
	EXP_COMP_M0_3,
	EXP_COMP_0,
	EXP_COMP_0_3,
	EXP_COMP_0_6,
	EXP_COMP_1_0,
	EXP_COMP_1_3,
	EXP_COMP_1_6,
	EXP_COMP_2
};

// Exposure Metering Mode

enum {
EXP_MODE_P = 0,
EXP_MODE_TV,
EXP_MODE_AV,
EXP_MODE_MANUAL,
EXP_MODE_BULB,
EXP_MODE_A_DEP,
EXP_MODE_DEP,
EXP_MODE_CUSTOM,
EXP_MODE_LOCK,
EXP_MODE_GREEN,
EXP_MODE_NIGHT_PORTRAIT,
EXP_MODE_SPORTS,
EXP_MODE_PORTRAIT,
EXP_MODE_LANDSCAPE,
EXP_MODE_CLOSEUP,
EXP_MODE_FLASH_OFF
};

// Release Button

enum {
RELEASE_NONE = 0,
RELEASE_PRESS_HALF,
RELEASE_PRESS_FULL,
RELEASE_RELEASE_HALF,
RELEASE_RELEASE_FULL
};

//==================================================================
// libgphoto2 call back funcs
//==================================================================

static void ctx_error_func (GPContext *context, const char *str, void *data)
{
	fprintf  (stderr, "\n*** Contexterror ***\n%s\n",str);
	fflush   (stderr);
}

static void ctx_status_func (GPContext *context, const char *str, void *data)
{
	fprintf  (stderr, "%s\n", str);
	fflush   (stderr);
}

//==================================================================
// config widget search
//==================================================================
static int lookup_widget(CameraWidget*widget, const char *key, CameraWidget **child) {
	int ret;
	ret = gp_widget_get_child_by_name (widget, key, child);
	if (ret < GP_OK)
		ret = gp_widget_get_child_by_label (widget, key, child);
	return ret;
}

//==================================================================
// capture on
//==================================================================

int canon_enable_capture (Camera *camera, int onoff, GPContext *context) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int			ret;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = lookup_widget (widget, "capture", &child);
	if (ret < GP_OK) {
		/*fprintf (stderr, "lookup widget failed: %d\n", ret);*/
		goto out;
	}

	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "gp_widget_get_type failed: %d\n", ret);
		goto out;
	}
	switch (type) {
        case GP_WIDGET_TOGGLE:
			break;
		default:
			fprintf (stderr, "widget has bad type %d\n", type);
			ret = GP_ERROR_BAD_PARAMETERS;
			goto out;
	}
	/* Now set the toggle to the wanted value */
	ret = gp_widget_set_value (child, &onoff);
	if (ret < GP_OK) {
		fprintf (stderr, "toggling Canon capture to %d failed with %d\n", onoff, ret);
		goto out;
	}
	/* OK */
	ret = gp_camera_set_config (camera, widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_set_config failed: %d\n", ret);
		return ret;
	}
out:
	gp_widget_free (widget);
	return ret;
}

//==================================================================
// Set Release Button
//==================================================================

int set_release(Camera *camera, GPContext *context, int button) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int			ret;
	float			rval;
	char			*mval;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = lookup_widget (widget, "eosremoterelease", &child);
	if (ret < GP_OK) {
		fprintf (stderr, "lookup 'eosremoterelease' failed: %d\n", ret);
		goto out_release;
	}

	/* check that this is a toggle */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out_release;
	}
	switch (type) {
        case GP_WIDGET_RADIO: {
		int choices = gp_widget_count_choices (child);

		ret = gp_widget_get_value (child, &mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget value: %d\n", ret);
			goto out_release;
		}
		printf("Current button: %s\n",mval);
		
		ret = gp_widget_get_choice (child, button, (const char**)&mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget choice %d: %d\n", button, ret);
			goto out_release;
		}
		printf("Setting button to %s\n",mval);
		
		ret = gp_widget_set_value (child, mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not set widget value : %d\n", ret);
			goto out_release;
		}
		
		break;
	}
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out_release;
	}
	
	ret = gp_camera_set_config (camera, widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "could not set config tree to target: %d\n", ret);
		goto out_release;
	}

out_release:
	gp_widget_free (widget);

	return ret;

}

//==================================================================
// Set Exposure Metering Mode
//==================================================================

int set_exp_mode(Camera *camera, GPContext *context, int mode) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int			ret;
	float			rval;
	char			*mval;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = lookup_widget (widget, "autoexposuremode", &child);
	if (ret < GP_OK) {
		fprintf (stderr, "lookup 'autoexposuremode' failed: %d\n", ret);
		goto out_exp_mode;
	}

	/* check that this is a toggle */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out_exp_mode;
	}
	switch (type) {
        case GP_WIDGET_RADIO: {
		int choices = gp_widget_count_choices (child);

		ret = gp_widget_get_value (child, &mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget value: %d\n", ret);
			goto out_exp_mode;
		}
		printf("Current mode: %s\n",mval);
				
		ret = gp_widget_get_choice (child, mode, (const char**)&mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget choice %d: %d\n", mode, ret);
			goto out_exp_mode;
		}
		printf("Setting mode to %s\n",mval);
		
		ret = gp_widget_set_value (child, mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not set widget value : %d\n", ret);
			goto out_exp_mode;
		}
		
		break;
	}
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out_exp_mode;
	}
	
	ret = gp_camera_set_config (camera, widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "could not set config tree to target: %d\n", ret);
		goto out_exp_mode;
	}

out_exp_mode:
	gp_widget_free (widget);

	return ret;

}

int get_exp_mode(Camera *camera, GPContext *context, char *mode_val) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int			ret;
	float			rval;
	char			*mval;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = lookup_widget (widget, "autoexposuremode", &child);
	if (ret < GP_OK) {
		fprintf (stderr, "lookup 'autoexposuremode' failed: %d\n", ret);
		goto out_get_mode;
	}

	/* check that this is a toggle */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out_get_mode;
	}
	switch (type) {
        case GP_WIDGET_RADIO: {
		int choices = gp_widget_count_choices (child);

		ret = gp_widget_get_value (child, mode_val);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget value: %d\n", ret);
			goto out_get_mode;
		}
		printf("Current mode: %s\n",mode_val);
		
		
		break;
	}
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out_get_mode;
	}
	
out_get_mode:
	gp_widget_free (widget);

	return ret;

}

//==================================================================
// Set Exposure Compensation
//==================================================================


int set_exp_comp(Camera *camera, GPContext *context, int comp_val) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int			ret;
	float			rval;
	char			*mval;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = lookup_widget (widget, "exposurecompensation", &child);
	if (ret < GP_OK) {
		fprintf (stderr, "lookup 'exposurecompensation' failed: %d\n", ret);
		goto out_iso;
	}

	/* check that this is a toggle */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out_iso;
	}
	switch (type) {
        case GP_WIDGET_RADIO: {
		int choices = gp_widget_count_choices (child);

		ret = gp_widget_get_value (child, &mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget value: %d\n", ret);
			goto out_iso;
		}
		printf("Current iso: %s\n",mval);
		
		ret = gp_widget_get_choice (child, comp_val, (const char**)&mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget choice %d: %d\n", comp_val, ret);
			goto out_iso;
		}
		printf("Setting exp comp to %s\n",mval);
		
		ret = gp_widget_set_value (child, mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not set widget value : %d\n", ret);
			goto out_iso;
		}
		
		break;
	}
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out_iso;
	}
	
	ret = gp_camera_set_config (camera, widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "could not set config tree to target: %d\n", ret);
		goto out_iso;
	}

out_iso:
	gp_widget_free (widget);

	return ret;

}

//==================================================================
// Set ISO
//==================================================================


int set_iso(Camera *camera, GPContext *context, int iso_value) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int			ret;
	float			rval;
	char			*mval;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = lookup_widget (widget, "iso", &child);
	if (ret < GP_OK) {
		fprintf (stderr, "lookup 'iso' failed: %d\n", ret);
		goto out_iso;
	}

	/* check that this is a toggle */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out_iso;
	}
	switch (type) {
        case GP_WIDGET_RADIO: {
		int choices = gp_widget_count_choices (child);

		ret = gp_widget_get_value (child, &mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget value: %d\n", ret);
			goto out_iso;
		}
		printf("Current iso: %s\n",mval);
		
		ret = gp_widget_get_choice (child, iso_value, (const char**)&mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget choice %d: %d\n", iso_value, ret);
			goto out_iso;
		}
		printf("Setting iso to %s\n",mval);
		
		ret = gp_widget_set_value (child, mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not set widget value to 1: %d\n", ret);
			goto out_iso;
		}
		
		break;
	}
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out_iso;
	}
	
	ret = gp_camera_set_config (camera, widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "could not set config tree to target: %d\n", ret);
		goto out_iso;
	}

out_iso:
	gp_widget_free (widget);

	return ret;

}

int get_iso(Camera *camera, GPContext *context, char *iso_val) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int			ret;
	float			rval;
	char			*mval;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = lookup_widget (widget, "iso", &child);
	if (ret < GP_OK) {
		fprintf (stderr, "lookup 'iso' failed: %d\n", ret);
		goto out_get_iso;
	}

	/* check that this is a toggle */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out_get_iso;
	}
	switch (type) {
        case GP_WIDGET_RADIO: {
		int choices = gp_widget_count_choices (child);

		ret = gp_widget_get_value (child, iso_val);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget value: %d\n", ret);
			goto out_get_iso;
		}
		printf("Current iso: %s\n",iso_val);
		
		
		break;
	}
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out_get_iso;
	}
	
out_get_iso:
	gp_widget_free (widget);

	return ret;

}

//==================================================================
// Set Aperture
//==================================================================


int set_aperture(Camera *camera, GPContext *context, int ap_value) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int			ret;
	float			rval;
	char			*mval;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = lookup_widget (widget, "aperture", &child);
	if (ret < GP_OK) {
		fprintf (stderr, "lookup 'aperture' failed: %d\n", ret);
		goto out_aperture;
	}

	/* check that this is a toggle */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out_aperture;
	}
	switch (type) {
        case GP_WIDGET_RADIO: {
		int choices = gp_widget_count_choices (child);

		ret = gp_widget_get_value (child, &mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget value: %d\n", ret);
			goto out_aperture;
		}
		printf("Current aperture: %s\n",mval);
		
		ret = gp_widget_get_choice (child, ap_value, (const char**)&mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget choice %d: %d\n", ap_value, ret);
			goto out_aperture;
		}
		printf("Setting aperture to %s\n",mval);
		
		ret = gp_widget_set_value (child, mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not set widget value to 1: %d\n", ret);
			goto out_aperture;
		}
		
		break;
	}
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out_aperture;
	}
	
	ret = gp_camera_set_config (camera, widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "could not set config tree to target: %d\n", ret);
		goto out_aperture;
	}

out_aperture:
	gp_widget_free (widget);

	return ret;

}

int get_aperture(Camera *camera, GPContext *context, char *aperture_val) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int			ret;
	float			rval;
	char			*mval;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = lookup_widget (widget, "aperture", &child);
	if (ret < GP_OK) {
		fprintf (stderr, "lookup 'aperture' failed: %d\n", ret);
		goto out_get_aperture;
	}

	/* check that this is a toggle */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out_get_aperture;
	}
	switch (type) {
        case GP_WIDGET_RADIO: {
		int choices = gp_widget_count_choices (child);

		ret = gp_widget_get_value (child, aperture_val);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget value: %d\n", ret);
			goto out_get_aperture;
		}
		printf("Current aperture: %s\n",aperture_val);
		
		
		break;
	}
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out_get_aperture;
	}
	
out_get_aperture:
	gp_widget_free (widget);

	return ret;

}

//==================================================================
// Set Shutter Speed
//==================================================================

int set_ss(Camera *camera, GPContext *context, int ss_index) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int			ret;
	float			rval;
	char			*mval;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = lookup_widget (widget, "shutterspeed", &child);
	if (ret < GP_OK) {
		fprintf (stderr, "lookup 'shutterspeed' failed: %d\n", ret);
		goto out_ss;
	}

	/* check that this is a toggle */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out_ss;
	}
	switch (type) {
        case GP_WIDGET_RADIO: {
		int choices = gp_widget_count_choices (child);

		ret = gp_widget_get_value (child, &mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget value: %d\n", ret);
			goto out_ss;
		}
		printf("Current ss: %s\n",mval);
		
		ret = gp_widget_get_choice (child, ss_index, (const char**)&mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget choice %d: %d\n", ss_index, ret);
			goto out_ss;
		}
		printf("Setting ss to %s\n",mval);
		
		ret = gp_widget_set_value (child, mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not set widget value to 1: %d\n", ret);
			goto out_ss;
		}
		
		break;
	}
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out_ss;
	}
	
	ret = gp_camera_set_config (camera, widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "could not set config tree to target: %d\n", ret);
		goto out_ss;
	}

out_ss:
	gp_widget_free (widget);

	return ret;

}

int get_ss(Camera *camera, GPContext *context, char *ss_val) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int			ret;
	float			rval;
	char			*mval;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = lookup_widget (widget, "shutterspeed", &child);
	if (ret < GP_OK) {
		fprintf (stderr, "lookup 'shutterspeed' failed: %d\n", ret);
		goto out_get_ss;
	}

	/* check that this is a toggle */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out_get_ss;
	}
	switch (type) {
        case GP_WIDGET_RADIO: {
		int choices = gp_widget_count_choices (child);

		ret = gp_widget_get_value (child, ss_val);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget value: %d\n", ret);
			goto out_get_ss;
		}
		printf("Current ss: %s\n",ss_val);
		
		
		break;
	}
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out_get_ss;
	}
	
out_get_ss:
	gp_widget_free (widget);

	return ret;

}

//==================================================================
// Switch Capture Target
//==================================================================

int set_capture_target(Camera *camera, GPContext *context, int target) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int			ret;
	float			rval;
	char			*mval;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = lookup_widget (widget, "capturetarget", &child);
	if (ret < GP_OK) {
		fprintf (stderr, "lookup 'capturetarget' failed: %d\n", ret);
		goto out_capture_target;
	}

	/* check that this is a toggle */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out_capture_target;
	}
	switch (type) {
        case GP_WIDGET_RADIO: {
		int choices = gp_widget_count_choices (child);

		ret = gp_widget_get_value (child, &mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget value: %d\n", ret);
			goto out_capture_target;
		}
		printf("Current capture target: %s\n",mval);
		
		ret = gp_widget_get_choice (child, target, (const char**)&mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget choice %d: %d\n", target, ret);
			goto out_capture_target;
		}
		printf("Setting capturetarget to %s\n",mval);
		
		ret = gp_widget_set_value (child, mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not set widget value to 1: %d\n", ret);
			goto out_capture_target;
		}
		
		break;
	}
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out_capture_target;
	}
	
	ret = gp_camera_set_config (camera, widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "could not set config tree to target: %d\n", ret);
		goto out_capture_target;
	}

out_capture_target:
	gp_widget_free (widget);

	return ret;

}

//==================================================================
//  Liveview on/off 
//==================================================================

int camera_eosviewfinder(Camera *camera, GPContext *context, int onoff) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int			ret,val;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}

	ret = lookup_widget (widget, "viewfinder", &child);
	if (ret < GP_OK) {
		fprintf (stderr, "lookup 'eosviewfinder' failed: %d\n", ret);
		goto out;
	}

	/* check that this is a toggle */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out;
	}
	switch (type) {
        case GP_WIDGET_TOGGLE:
		break;
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out;
	}

	ret = gp_widget_get_value (child, &val);
	if (ret < GP_OK) {
		fprintf (stderr, "could not get widget value: %d\n", ret);
		goto out;
	}
	
	val = onoff;
	ret = gp_widget_set_value (child, &val);
	if (ret < GP_OK) {
		fprintf (stderr, "could not set widget value to 1: %d\n", ret);
		goto out;
	}

	ret = gp_camera_set_config (camera, widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "could not set config tree to eosviewfinder: %d\n", ret);
		goto out;
	}
out:
	gp_widget_free (widget);
	return ret;
}

//==================================================================
// Auto Focus on/off
//==================================================================

int camera_auto_focus(Camera *camera, GPContext *context, int onoff) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int			ret,val;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = lookup_widget (widget, "autofocusdrive", &child);
	if (ret < GP_OK) {
		fprintf (stderr, "lookup 'autofocusdrive' failed: %d\n", ret);
		goto out;
	}

	/* check that this is a toggle */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out;
	}
	switch (type) {
        case GP_WIDGET_TOGGLE:
		break;
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out;
	}

	ret = gp_widget_get_value (child, &val);
	if (ret < GP_OK) {
		fprintf (stderr, "could not get widget value: %d\n", ret);
		goto out;
	}

	val = onoff;

	ret = gp_widget_set_value (child, &val);
	if (ret < GP_OK) {
		fprintf (stderr, "could not set widget value to 1: %d\n", ret);
		goto out;
	}

	ret = gp_camera_set_config (camera, widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "could not set config tree to autofocus: %d\n", ret);
		goto out;
	}
out:
	gp_widget_free (widget);
	return ret;
}

//==================================================================
// Manual Focusing 
//==================================================================
int camera_manual_focus (Camera *camera, int xx, GPContext *context) {
	CameraWidget		*widget = NULL, *child = NULL;
	CameraWidgetType	type;
	int			ret;
	float			rval;
	char			*mval;

	ret = gp_camera_get_config (camera, &widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "camera_get_config failed: %d\n", ret);
		return ret;
	}
	ret = lookup_widget (widget, "manualfocusdrive", &child);
	if (ret < GP_OK) {
		fprintf (stderr, "lookup 'manualfocusdrive' failed: %d\n", ret);
		goto out;
	}

	/* check that this is a toggle */
	ret = gp_widget_get_type (child, &type);
	if (ret < GP_OK) {
		fprintf (stderr, "widget get type failed: %d\n", ret);
		goto out;
	}
	switch (type) {
        case GP_WIDGET_RADIO: {
		int choices = gp_widget_count_choices (child);

		ret = gp_widget_get_value (child, &mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not get widget value: %d\n", ret);
			goto out;
		}
		
		if (choices == 7) { /* see what Canon has in EOS_MFDrive */
			ret = gp_widget_get_choice (child, xx, (const char**)&mval);
			if (ret < GP_OK) {
				fprintf (stderr, "could not get widget choice %d: %d\n", xx, ret);
				goto out;
			}
			fprintf(stderr,"manual focus %d -> %s\n", xx, mval);
		}
		ret = gp_widget_set_value (child, mval);
		if (ret < GP_OK) {
			fprintf (stderr, "could not set widget value to 1: %d\n", ret);
			goto out;
		}
		break;
	}
	default:
		fprintf (stderr, "widget has bad type %d\n", type);
		ret = GP_ERROR_BAD_PARAMETERS;
		goto out;
	}


	ret = gp_camera_set_config (camera, widget, context);
	if (ret < GP_OK) {
		fprintf (stderr, "could not set config tree to autofocus: %d\n", ret);
		goto out;
	}
out:
	gp_widget_free (widget);
	return ret;
}

//==================================================================
// Tethering
//==================================================================
static void camera_tether(Camera *camera, GPContext *context) {
	int fd, retval;
	CameraFile *file;
	CameraEventType	evttype;
	CameraFilePath	*path;
	void	*evtdata;
	int waittime = 2000;
	printf("Tethering...\n");
	
	while (1) {
		retval = gp_camera_wait_for_event (camera, waittime, &evttype, &evtdata, context);
		if (retval != GP_OK)
			break;
		switch (evttype) {
		case GP_EVENT_FILE_ADDED:
			path = (CameraFilePath*)evtdata;
			printf("File added on the camera: %s/%s\n", path->folder, path->name);
			fd = open(path->name, O_CREAT | O_WRONLY, 0644);
			retval = gp_file_new_from_fd(&file, fd);
			printf("  Downloading %s...\n", path->name);
			retval = gp_camera_file_get(camera, path->folder, path->name,
				     GP_FILE_TYPE_NORMAL, file, context);
			printf("  Deleting %s on camera...\n", path->name);
			retval = gp_camera_file_delete(camera, path->folder, path->name, context);
			gp_file_free(file);
			return;
			break;
		case GP_EVENT_FOLDER_ADDED:
			path = (CameraFilePath*)evtdata;
			printf("Folder added on camera: %s / %s\n", path->folder, path->name);
			break;
		case GP_EVENT_CAPTURE_COMPLETE:
			printf("Capture Complete.\n");
			waittime = 100;
			break;
		case GP_EVENT_TIMEOUT:
			printf("Timeout.\n");
			return;
			break;
		case GP_EVENT_UNKNOWN:
			//if (evtdata) {
			//	printf("Unknown event: %s.\n", (char*)evtdata);
			//} else {
			//	printf("Unknown event.\n");
			//}
			break;
		default:
			printf("evttype = %d?\n", evttype);
			break;
		}
	}
}

//==================================================================
// open camera
//==================================================================

int open_camera(Camera** camera, CameraAbilitiesList* driverlist, GPPortInfoList* portlist, const char *model, const char *port) 
{
	int		ret, m, p;
	CameraAbilities	a;
	GPPortInfo	pi;

	ret = gp_camera_new (camera);
	if (ret < GP_OK) 
		return ret;

	/* First lookup the model / driver */
	m = gp_abilities_list_lookup_model (driverlist, model);
	if (m < GP_OK) 
		return ret;
	ret = gp_abilities_list_get_abilities (driverlist, m, &a);
	if (ret < GP_OK) 
		return ret;
	ret = gp_camera_set_abilities (*camera, a);
	if (ret < GP_OK) 
		return ret;

	/* Then associate the camera with the specified port */
	p = gp_port_info_list_lookup_path (portlist, port);  
	ret = gp_port_info_list_get_info (portlist, p, &pi);  
	if (ret < GP_OK) 
		return ret;	
	ret = gp_camera_set_port_info (*camera, pi);
	if (ret < GP_OK) 
		return ret;

	return GP_OK;
}

//==================================================================
// save file to host
//==================================================================
void save_file_to_host(Camera *camera, GPContext *context,CameraFilePath &camera_file_path) {
	CameraFile* camerafile;
	camerafile = NULL;
	CameraFilePath* curpath;
	curpath = NULL;
	int fd;
	fd = 0;
	char save_filename[260];
	int iexpose;

	curpath = &camera_file_path; 			
	sprintf(save_filename,"%s",curpath->name);
	fd = open(save_filename, O_CREAT | O_WRONLY, 0644);
	gp_file_new_from_fd(&camerafile, fd);
	gp_camera_file_get(camera, curpath->folder, curpath->name, GP_FILE_TYPE_NORMAL, camerafile, context);
	gp_file_free(camerafile);
	gp_camera_file_delete(camera, curpath->folder, curpath->name, context);
	close(fd);

}

//==================================================================
// main
//==================================================================

int main(int argc, char **argv) {
	int	retval;
	GPPortInfoList* portlist = NULL;
	CameraAbilitiesList* driverlist = NULL;

	CameraList* camlist = NULL;
	int numcams = 0;
	Camera* cams[MAX_CAMERAS] = { NULL, NULL };
	const char* name = NULL;
	const char* value = NULL;
	int i = 0;
	char msg[128];
	clock_t c1,c2;
	
	GPContext *canoncontext;
	canoncontext = gp_context_new();
    gp_context_set_error_func (canoncontext, ctx_error_func, NULL);
    gp_context_set_status_func (canoncontext, ctx_status_func, NULL);

	gp_port_info_list_new (&portlist);
	gp_port_info_list_load (portlist);
	gp_port_info_list_count (portlist);

	gp_abilities_list_new (&driverlist);
	gp_abilities_list_load (driverlist, canoncontext);

	gp_list_new (&camlist);
	gp_abilities_list_detect (driverlist, portlist, camlist, canoncontext);
	numcams = gp_list_count(camlist);

	/* list cameras */
	printf("cameras detected: %d\n", numcams);
	for (i = 0; i < numcams; i++) 
	{
		gp_list_get_name  (camlist, i, &name);
		gp_list_get_value (camlist, i, &value);
		printf("camera[%d]: %s, %s\n", i + 1, name, value);
	}

	/* open cameras */
	for (i = 0; i < numcams; i++)  	{ 		
		gp_list_get_name  (camlist, i, &name); 		
		gp_list_get_value (camlist, i, &value); 		
		printf("opening camera[%d]: %s, %s...\n", i + 1, name, value); 		
		if (open_camera (&cams[i], driverlist, portlist, name, value) < GP_OK)
		{
			printf("failed to open camera(s)\n");
		}
	}
	
	if (numcams < 1)
	{
		printf("No camera(s) found.\n");
		exit(-2);
	}
	
	/* init etc */
	retval = gp_camera_init(cams[0], canoncontext);

	if (retval != GP_OK) {
		printf("  Retval: %d\n", retval);
		exit (1);
	}
	canon_enable_capture(cams[0], TRUE, canoncontext);
	
	/* set config values to initial */
	set_exp_mode(cams[0], canoncontext,EXP_MODE_MANUAL);
	
	set_capture_target(cams[0], canoncontext,1); // CF card
	//set_capture_target(cams[0], canoncontext,0); // internal DRAM
	
	set_ss(cams[0], canoncontext,SS_1_3);
	//set_aperture(cams[0], canoncontext, APERTURE_4);
	set_iso(cams[0], canoncontext, ISO_800);
	
	
	CameraFilePath camera_file_path;
	
	for (i=0;i<3;i++) {
		CameraFile *file;
		char output_file[32];
		time_t start_time,end_time;
		double sec;
		
#ifdef DO_TETHER_CAPTURE
		/* Triggered Capture */
		retval = gp_camera_trigger_capture (cams[0], canoncontext);
		if ((retval != GP_OK) && (retval != GP_ERROR) && (retval != GP_ERROR_CAMERA_BUSY)) {
			fprintf(stderr,"triggering capture had error %d\n", retval);
			goto skip;
		}
		camera_tether(cams[0], canoncontext) ;
#endif

skip:
		time(&start_time);
		
		// shot 1
		set_ss(cams[0], canoncontext,SS_4);
		if (gp_camera_capture(cams[0], GP_CAPTURE_IMAGE, &camera_file_path, canoncontext) < GP_OK)
		{
			printf("capture failed\n");
		}
		printf("Info: Captured to file %s/%s\n",camera_file_path.folder,camera_file_path.name);
#ifdef GET_FILE
		save_file_to_host(cams[0], canoncontext,camera_file_path);
#endif

		// shot 2
		set_ss(cams[0], canoncontext,SS_1p4);
		if (gp_camera_capture(cams[0], GP_CAPTURE_IMAGE, &camera_file_path, canoncontext) < GP_OK)
		{
			printf("capture failed\n");
		}
		printf("Info: Captured to file %s/%s\n",camera_file_path.folder,camera_file_path.name);
#ifdef GET_FILE
		save_file_to_host(cams[0], canoncontext,camera_file_path);
#endif
		
		// shot 3
		set_ss(cams[0], canoncontext,SS_1p60);
		if (gp_camera_capture(cams[0], GP_CAPTURE_IMAGE, &camera_file_path, canoncontext) < GP_OK)
		{
			printf("capture failed\n");
		}
		printf("Info: Captured to file %s/%s\n",camera_file_path.folder,camera_file_path.name);
#ifdef GET_FILE
		save_file_to_host(cams[0], canoncontext,camera_file_path);
#endif
		
		// wait interval
		while(1) {
			time(&end_time);
			sec = difftime(end_time,start_time);
			if(sec >= sInterval)
				break;
		}
	}

	sleep(2);
	
	for (i = 0; i < numcams; i++)  	{ 		
		gp_camera_exit(cams[i], canoncontext);
	}
	printf("Bye\n");
	
	return 0;
}
