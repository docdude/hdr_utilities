#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <math.h>

typedef struct DisplayChromacities
{
	double RedX;
	double RedY;
	double GreenX;
	double GreenY;
	double BlueX;
	double BlueY;
	double WhiteX;
	double WhiteY;
} DisplayChromacities;

#define METADATA_SCALING 50000

static const DisplayChromacities DisplayChromacityList[] =
{
	{ 0.64000, 0.33000, 0.30000, 0.60000, 0.15000, 0.06000, 0.31270, 0.32900 }, // Display Gamut Rec709
	{ 0.70800, 0.29200, 0.17000, 0.79700, 0.13100, 0.04600, 0.31270, 0.32900 }, // Display Gamut Rec2020
	{ 0.68000, 0.32000, 0.26500, 0.69000, 0.15000, 0.06000, 0.31270, 0.32900 }, // Display Gamut P3D65
	{ 0.68000, 0.32000, 0.26500, 0.69000, 0.15000, 0.06000, 0.31400, 0.35100 }, // Display Gamut P3DCI(Theater)
	{ 0.68000, 0.32000, 0.26500, 0.69000, 0.15000, 0.06000, 0.32168, 0.33767 }, // Display Gamut P3D60(ACES Cinema)
	{ 0.67030, 0.32970, 0.26060, 0.67320, 0.14420, 0.05120, 0.31270, 0.32900 }, //videoforge dovi ??
};

int usage(const char* name)
{
	printf("\n");
	printf("      %s <eotf> <hdr_primaries> <Max luma (x Nits)> <Min luma (0.000x Nits)> <Max Cll (x nits)> <Max Fall (x Nits)> \n", name);
	printf("\n");
	printf("        HDR metadata is in nits = candella per square meter (cd/m^2)			 \n");
	printf("	  Max Luma (1 - 65535)\n");
	printf("	  Min Luma (1 - 65535) unsigned 16-bit value in units of 0.0001 cd/m^2, where 1 represents 0.0001 cd/m^2 and 0xFFFF represents 6.5535 cd/m^2\n");
	printf("	  MaxCLL   (1 - 65535)\n");
	printf("	  MaxFall  (1 - 65535)\n");
	printf("\n");
	printf("        <eotf>\n");
	printf("	  Traditional Gamma-SDR Luminance Range = 0\n");
	printf("	  raditional Gamma-HDR Luminance Range  = 1\n");
	printf("	  SMPTE ST 2084 			= 2\n");
	printf("	  Hybrid Log-Gamma (HLG) 		= 3\n");
	printf("	  Reserved for future use 		= 4\n");
	printf("	  Reserved for future use 		= 5\n");
	printf("\n");
	printf("        <hdr_primaries> \n");
	printf("	  Display Gamut Rec709        		= 0\n");
	printf("	  Display Gamut Rec2020      		= 1\n");
	printf("	  Display Gamut P3D65         		= 2\n");
	printf("	  Display Gamut P3DCI(Theater)		= 3\n");
	printf("	  Display Gamut P3D60(ACES Cinema)	= 4\n");

	printf("\n");
	printf("Example:\n");
	printf("  %s 0 2 10000 1 10000 250 ", name);
	printf(" ==>  Sets HDR metadata to: 2 <ST2084>, 2 <P3D65>, 10000 <Max Luma>, 1 <Min Luma>, 10000 <MaxCll>, 250 <MaxFall>, \n");
	return 0;
}
 
static uint8_t hdmi_infoframe_checksum(const uint8_t *ptr, size_t size)
{
	uint8_t csum = 0;
	size_t i;

	/* compute checksum */
	for (i = 0; i < size; i++)
		csum += ptr[i];

	return 256 - csum;
}

/* 
 * Data Bytes 21 – 22 specify a value for the min_display_mastering_luminance. This value is coded as an
 * zero and 0xC350 represents 1.0000
 * unsigned 16-bit value in units of 0.0001 cd/m2, where 0x0001 represents 0.0001 cd/m2 and 0xFFFF
 * represents 6.5535 cd/m2.
 *
 * Data Bytes 23 – 24 contain the Maximum Content Light Level (MaxCLL). This value is coded as an
 * unsigned 16-bit value in units of 1 cd/m2, where 0x0001 represents 1 cd/m2 and 0xFFFF represents
 * 65535 cd/m2.
 *
 * Data Bytes 25 – 26 contain the Maximum Frame-Average Light Level (MaxFALL). This value is coded as
 * an unsigned 16-bit value in units of 1 cd/m2, where 0x0001 represents 1 cd/m2 and 0xFFFF represents
 * 65535 cd/m2.
*/
int main(int argc, char **argv){

	int head1=0x87;
	int head2=0x01;
	size_t length=0x1A;
	int checksum=0x00;
	int byte1=0x2; //P3D65
	int byte2=0x00;//metadata type

	uint16_t idx, green_x, green_y, blue_x, blue_y, red_x, red_y, wp_x, wp_y, max_luma, min_luma, max_cll, max_fall;
	uint8_t *hdr_metadata = NULL;

	hdr_metadata = malloc(50);
	if (hdr_metadata == NULL) {
		fprintf(stderr, "Failed to allocate memory\n");
	}

	if(argc < 2)
		return usage(argv[0]);
	if(argc > 1) {
		byte1 = atoi(argv[1]);
		idx = atoi(argv[2]);
		hdr_metadata[0]=head1;
		hdr_metadata[1]=head2;
		hdr_metadata[2]=length;
		hdr_metadata[3]=checksum;
		hdr_metadata[4]=byte1;
		hdr_metadata[5]=byte2;

		green_x = round(DisplayChromacityList[idx].GreenX * METADATA_SCALING); //Green Point X (Byte3,4) 
		hdr_metadata[6]=green_x & 0xff;
		hdr_metadata[7]=green_x >> 8;
		
		green_y = round(DisplayChromacityList[idx].GreenY * METADATA_SCALING);//Green Point Y (Byte5,6)
		hdr_metadata[8]=green_y & 0xff;
		hdr_metadata[9]=green_y >> 8;
		
		blue_x = round(DisplayChromacityList[idx].BlueX * METADATA_SCALING); //Blue Point X (Byte7,8)
		hdr_metadata[10]=blue_x & 0xff;
		hdr_metadata[11]=blue_x >> 8;
		
		blue_y = round(DisplayChromacityList[idx].BlueY * METADATA_SCALING);//Blue Point Y (Byte9,10)
		hdr_metadata[12]=blue_y & 0xff;
		hdr_metadata[13]=blue_y >> 8;

		red_x = round(DisplayChromacityList[idx].RedX * METADATA_SCALING);//Red Point X (Byte11,12)
		hdr_metadata[14]=red_x & 0xff;
		hdr_metadata[15]=red_x >> 8;

		red_y = round(DisplayChromacityList[idx].RedY * METADATA_SCALING);	//Red Point Y (Byte13,14)
		hdr_metadata[16]=red_y & 0xff;
		hdr_metadata[17]=red_y >> 8;

		wp_x = round(DisplayChromacityList[idx].WhiteX * METADATA_SCALING);//White Point X(Byte15,16)
		hdr_metadata[18]=wp_x & 0xff;
		hdr_metadata[19]=wp_x >> 8;

		wp_y = round(DisplayChromacityList[idx].WhiteY * METADATA_SCALING);//White Point Y(Byte17,18)
		hdr_metadata[20]=wp_y & 0xff;
		hdr_metadata[21]=wp_y >> 8;

		max_luma = atof(argv[3]);//(uint16_t)(10000.0f * 10000.0f);//MD Peak (Byte19,20)
		hdr_metadata[22]=max_luma & 0xff;
		hdr_metadata[23]=max_luma >> 8;

		min_luma = (atof(argv[4]) * 10000.0f)/10000.0f;//MD Black (Byte21,22)
		hdr_metadata[24]=min_luma & 0xff;
		hdr_metadata[25]=min_luma >> 8;

		max_cll = atof(argv[5]);//10000.0f;//Max CLL  (Byte23,24) 
		hdr_metadata[26]=max_cll & 0xff;
		hdr_metadata[27]=max_cll >> 8;

		max_fall = atof(argv[6]);// 400.0f;//Max FALL (Byte25,26)
		hdr_metadata[28]=max_fall & 0xff;
		hdr_metadata[29]=max_fall >> 8;

 }

	checksum = hdmi_infoframe_checksum(hdr_metadata, length+4); //length + Infoframe header bytes

	printf ("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			head1, head2, length, checksum, byte1, byte2,  green_x & 0xff, green_x >> 8, green_y & 0xff, green_y >> 8,  blue_x & 0xff, blue_x >> 8,
			blue_y & 0xff, blue_y >> 8, red_x & 0xff, red_x >> 8, red_y & 0xff, red_y >> 8, wp_x & 0xff, wp_x >> 8, wp_y & 0xff, wp_y >> 8, 
			max_luma & 0xff, max_luma >> 8, min_luma & 0xff, min_luma >> 8,  max_cll & 0xff, max_cll >> 8, max_fall & 0xff, max_fall >> 8);

	free(hdr_metadata);
}










