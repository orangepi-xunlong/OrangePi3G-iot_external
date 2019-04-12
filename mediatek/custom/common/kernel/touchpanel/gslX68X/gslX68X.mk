obj-y += gsl_point_id
$(obj)/gsl_point_id: $(srctree)/$(obj)/touchpanel/gsl_point_id
	cp $(srctree)/$(obj)/touchpanel/gsl_point_id $(obj)/gsl_point_id
