#include "math.h"

const u16 IntRcp_Table[] /*IWRAM_DATA*/ = {
	0xffff, 0xffff, 0x7fff, 0x5555, 0x3fff, 0x3333, 0x2aaa, 0x2492, 0x1fff, 0x1c71, 
	0x1999, 0x1745, 0x1555, 0x13b1, 0x1249, 0x1111, 0x0fff, 0x0f0f, 0x0e38, 0x0d79, 
	0x0ccc, 0x0c30, 0x0ba2, 0x0b21, 0x0aaa, 0x0a3d, 0x09d8, 0x097b, 0x0924, 0x08d3, 
	0x0888, 0x0842, 0x07ff, 0x07c1, 0x0787, 0x0750, 0x071c, 0x06eb, 0x06bc, 0x0690, 
	0x0666, 0x063e, 0x0618, 0x05f4, 0x05d1, 0x05b0, 0x0590, 0x0572, 0x0555, 0x0539, 
	0x051e, 0x0505, 0x04ec, 0x04d4, 0x04bd, 0x04a7, 0x0492, 0x047d, 0x0469, 0x0456, 
	0x0444, 0x0432, 0x0421, 0x0410, 0x03ff, 0x03f0, 0x03e0, 0x03d2, 0x03c3, 0x03b5, 
	0x03a8, 0x039b, 0x038e, 0x0381, 0x0375, 0x0369, 0x035e, 0x0353, 0x0348, 0x033d, 
	0x0333, 0x0329, 0x031f, 0x0315, 0x030c, 0x0303, 0x02fa, 0x02f1, 0x02e8, 0x02e0, 
	0x02d8, 0x02d0, 0x02c8, 0x02c0, 0x02b9, 0x02b1, 0x02aa, 0x02a3, 0x029c, 0x0295, 
	0x028f, 0x0288, 0x0282, 0x027c, 0x0276, 0x0270, 0x026a, 0x0264, 0x025e, 0x0259, 
	0x0253, 0x024e, 0x0249, 0x0243, 0x023e, 0x0239, 0x0234, 0x0230, 0x022b, 0x0226, 
	0x0222, 0x021d, 0x0219, 0x0214, 0x0210, 0x020c, 0x0208, 0x0204, 0x01ff, 0x01fc, 
	0x01f8, 0x01f4, 0x01f0, 0x01ec, 0x01e9, 0x01e5, 0x01e1, 0x01de, 0x01da, 0x01d7, 
	0x01d4, 0x01d0, 0x01cd, 0x01ca, 0x01c7, 0x01c3, 0x01c0, 0x01bd, 0x01ba, 0x01b7, 
	0x01b4, 0x01b2, 0x01af, 0x01ac, 0x01a9, 0x01a6, 0x01a4, 0x01a1, 0x019e, 0x019c, 
	0x0199, 0x0197, 0x0194, 0x0192, 0x018f, 0x018d, 0x018a, 0x0188, 0x0186, 0x0183, 
	0x0181, 0x017f, 0x017d, 0x017a, 0x0178, 0x0176, 0x0174, 0x0172, 0x0170, 0x016e, 
	0x016c, 0x016a, 0x0168, 0x0166, 0x0164, 0x0162, 0x0160, 0x015e, 0x015c, 0x015a, 
	0x0158, 0x0157, 0x0155, 0x0153, 0x0151, 0x0150, 0x014e, 0x014c, 0x014a, 0x0149, 
	0x0147, 0x0146, 0x0144, 0x0142, 0x0141, 0x013f, 0x013e, 0x013c, 0x013b, 0x0139, 
	0x0138, 0x0136, 0x0135, 0x0133, 0x0132, 0x0130, 0x012f, 0x012e, 0x012c, 0x012b, 
	0x0129, 0x0128, 0x0127, 0x0125, 0x0124, 0x0123, 0x0121, 0x0120, 0x011f, 0x011e, 
	0x011c, 0x011b, 0x011a, 0x0119, 0x0118, 0x0116, 0x0115, 0x0114, 0x0113, 0x0112, 
	0x0111, 0x010f, 0x010e, 0x010d, 0x010c, 0x010b, 0x010a, 0x0109, 0x0108, 0x0107, 
	0x0106, 0x0105, 0x0104, 0x0103, 0x0102, 0x0101, };

const int rcp_table[512] = {
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff,
	0xffff8100, 0xff018180, 0xfe048280, 0xfd098380, 0xfc108480, 0xfb198580, 0xfa248600, 0xf9308700,
	0xf83e8880, 0xf74f8900, 0xf6618980, 0xf5748b00, 0xf48a8b80, 0xf3a18c80, 0xf2ba8d80, 0xf1d58e00,
	0xf0f18f00, 0xf00f9000, 0xef2f9080, 0xee509180, 0xed739280, 0xec989300, 0xebbe9400, 0xeae69480,
	0xea0f9580, 0xe93a9600, 0xe8669700, 0xe7949780, 0xe6c39880, 0xe5f49900, 0xe5269a00, 0xe45a9a80,
	0xe38f9b00, 0xe2c59c00, 0xe1fd9c80, 0xe1369d80, 0xe0719e00, 0xdfad9e80, 0xdeea9f00, 0xde28a000,
	0xdd68a080, 0xdca9a180, 0xdbeca200, 0xdb30a200, 0xda74a380, 0xd9bba380, 0xd902a480, 0xd84ba500,
	0xd795a580, 0xd6e0a600, 0xd62ca680, 0xd579a780, 0xd4c8a800, 0xd418a800, 0xd368a900, 0xd2baaa00,
	0xd20eaa00, 0xd162aa80, 0xd0b7ab00, 0xd00dac00, 0xcf65ac00, 0xcebdad00, 0xce17ad80, 0xcd72ad80,
	0xcccdae80, 0xcc2aaf00, 0xcb88af00, 0xcae6b000, 0xca46b080, 0xc9a7b080, 0xc908b180, 0xc86bb180,
	0xc7ceb280, 0xc733b280, 0xc698b380, 0xc5ffb380, 0xc566b400, 0xc4ceb500, 0xc438b500, 0xc3a2b580,
	0xc30db580, 0xc278b680, 0xc1e5b700, 0xc153b700, 0xc0c1b780, 0xc030b880, 0xbfa1b880, 0xbf12b880,
	0xbe83b980, 0xbdf6ba00, 0xbd6aba00, 0xbcdeba80, 0xbc53bb00, 0xbbc9bb00, 0xbb3fbc00, 0xbab7bc00,
	0xba2fbc80, 0xb9a8bd00, 0xb922bd00, 0xb89cbd80, 0xb817be00, 0xb793be80, 0xb710bf00, 0xb68ebf00,
	0xb60cbf80, 0xb58bbf80, 0xb50ac080, 0xb48bc080, 0xb40cc080, 0xb38dc180, 0xb310c180, 0xb293c200,
	0xb217c200, 0xb19bc280, 0xb120c300, 0xb0a6c380, 0xb02dc380, 0xafb4c380, 0xaf3bc480, 0xaec4c480,
	0xae4dc480, 0xadd6c580, 0xad61c580, 0xacecc580, 0xac77c600, 0xac03c680, 0xab90c680, 0xab1dc700,
	0xaaabc780, 0xaa3ac780, 0xa9c9c780, 0xa958c880, 0xa8e9c880, 0xa87ac880, 0xa80bc900, 0xa79dc900,
	0xa72fca00, 0xa6c3c980, 0xa656ca00, 0xa5eaca80, 0xa57fca80, 0xa514cb00, 0xa4aacb80, 0xa441cb80,
	0xa3d8cb80, 0xa36fcc00, 0xa307cc00, 0xa29fcc80, 0xa238cd00, 0xa1d2cd00, 0xa16ccd00, 0xa106cd80,
	0xa0a1ce00, 0xa03dce00, 0x9fd9ce00, 0x9f75ce80, 0x9f12ce80, 0x9eafcf00, 0x9e4dcf00, 0x9debcf80,
	0x9d8ad000, 0x9d2acf80, 0x9cc9d080, 0x9c6ad000, 0x9c0ad080, 0x9babd100, 0x9b4dd100, 0x9aefd100,
	0x9a91d180, 0x9a34d200, 0x99d8d180, 0x997bd280, 0x9920d200, 0x98c4d280, 0x9869d300, 0x980fd300,
	0x97b5d300, 0x975bd380, 0x9702d380, 0x96a9d380, 0x9650d400, 0x95f8d480, 0x95a1d400, 0x9549d500,
	0x94f3d480, 0x949cd500, 0x9446d500, 0x93f0d580, 0x939bd580, 0x9346d600, 0x92f2d580, 0x929dd680,
	0x924ad600, 0x91f6d680, 0x91a3d700, 0x9151d680, 0x90fed700, 0x90acd780, 0x905bd700, 0x9009d800,
	0x8fb9d780, 0x8f68d800, 0x8f18d800, 0x8ec8d880, 0x8e79d880, 0x8e2ad880, 0x8ddbd880, 0x8d8cd900,
	0x8d3ed980, 0x8cf1d900, 0x8ca3d980, 0x8c56d980, 0x8c09da00, 0x8bbdda00, 0x8b71da00, 0x8b25da00,
	0x8ad9da80, 0x8a8eda80, 0x8a43db00, 0x89f9db00, 0x89afdb00, 0x8965db00, 0x891bdb80, 0x88d2db80,
	0x8889db80, 0x8840dc00, 0x87f8dc00, 0x87b0dc00, 0x8768dc80, 0x8721dc80, 0x86dadc80, 0x8693dc80,
	0x864cdd00, 0x8606dd00, 0x85c0dd00, 0x857add80, 0x8535dd00, 0x84efdd80, 0x84aade00, 0x8466de00,
	0x8422dd80, 0x83ddde80, 0x839ade00, 0x8356de80, 0x8313de80, 0x82d0de80, 0x828ddf00, 0x824bdf00,
	0x8209df00, 0x81c7df00, 0x8185df80, 0x8144df00, 0x8102e000, 0x80c2df80, 0x8081e000, 0x8041df80,
};

const int rsq_table[512] = {
	0x0afa5e80, 0xe5b7aa00, 0x2f0b0080, 0xff0c5d00, 0x51c64a00, 0xc65a0700, 0x4e683380, 0x22cf6700,
	0xff9d6880, 0xe26ea600, 0xc9ba5e00, 0xb476b900, 0xa1e8d000, 0x9188b400, 0x42f07200, 0xf5d41200,
	0xe9f89980, 0xdf2b0e80, 0xd5487380, 0xcc2fcb80, 0xc3c61a00, 0xbbfa5e80, 0xb4b79b80, 0xadeed280,
	0xa7930400, 0xa19b3080, 0x9bfc5880, 0x96ad7d80, 0x91a89f00, 0x8ce6bd80, 0x8861d980, 0x8414f400,
	0x7ffc0c00, 0x7c142200, 0x78583680, 0x74c54a00, 0x71595b80, 0x6e106c80, 0x6ae97c00, 0x67e18a80,
	0x64f69880, 0x6227a580, 0x5f72b180, 0x5cd5bd00, 0x5a4fc800, 0x57dfd200, 0x5583db80, 0x533ae500,
	0x5104ed80, 0x4edff600, 0x4ccbfd80, 0x4ac60580, 0x48d10c80, 0x46ea1300, 0x45101980, 0x43432000,
	0x41832580, 0x3fce2b80, 0x3e253100, 0x3c873600, 0x3af33b00, 0x39694000, 0x37e94480, 0x36724880,
	0x35034d80, 0x339e5100, 0x32405500, 0x30ea5900, 0x2f9c5c80, 0x2e556000, 0x2d156380, 0x2bdc6680,
	0x2aa96a00, 0x297d6d00, 0x28577000, 0x27377280, 0x261c7580, 0x25077800, 0x23f77b00, 0x22ed7d00,
	0x21e78000, 0x20e78200, 0x1feb8400, 0x1ef38680, 0x1e008900, 0x1d128a80, 0x1c278d00, 0x1b418e80,
	0x1a5e9100, 0x19809280, 0x18a59480, 0x17ce9600, 0x16fa9780, 0x16299980, 0x155c9b80, 0x14939c80,
	0x13cc9e00, 0x1308a000, 0x1248a100, 0x118aa300, 0x10d0a400, 0x1018a500, 0x0f62a700, 0x0eb0a800,
	0x0e00a900, 0x0d52aa80, 0x0ca7ab80, 0x0bfead00, 0x0b58ae00, 0x0ab4af00, 0x0a12b080, 0x0973b100,
	0x08d5b280, 0x083ab380, 0x07a1b400, 0x0709b580, 0x0674b680, 0x05e1b700, 0x054fb800, 0x04bfb980,
	0x0432b980, 0x03a5bb00, 0x031bbb80, 0x0292bc80, 0x020bbd80, 0x0186be00, 0x0102bf00, 0x0080bf80,
	0x7fffc080, 0x7f80c100, 0x7f02c200, 0x7e86c280, 0x7e0bc300, 0x7d91c400, 0x7d19c480, 0x7ca2c580,
	0x7c2dc580, 0x7bb8c680, 0x7b45c780, 0x7ad4c780, 0x7a63c880, 0x79f4c900, 0x7986c980, 0x7919ca00,
	0x78adca80, 0x7842cb80, 0x77d9cb80, 0x7770cc80, 0x7709cc80, 0x76a2cd80, 0x763dcd80, 0x75d8ce80,
	0x7575ce80, 0x7512cf80, 0x74b1cf80, 0x7450d080, 0x73f1d080, 0x7392d100, 0x7334d200, 0x72d8d200,
	0x727cd200, 0x7220d300, 0x71c6d380, 0x716dd380, 0x7114d400, 0x70bcd480, 0x7065d500, 0x700fd500,
	0x6fb9d580, 0x6f64d600, 0x6f10d680, 0x6ebdd700, 0x6e6bd700, 0x6e19d780, 0x6dc8d780, 0x6d77d880,
	0x6d28d880, 0x6cd9d880, 0x6c8ad900, 0x6c3cd980, 0x6befda00, 0x6ba3da00, 0x6b57da80, 0x6b0cda80,
	0x6ac1db00, 0x6a77db80, 0x6a2edb80, 0x69e5dc00, 0x699ddc00, 0x6955dc80, 0x690edd00, 0x68c8dd00,
	0x6882dd80, 0x683ddd80, 0x67f8dd80, 0x67b3de80, 0x6770de00, 0x672cde80, 0x66e9df00, 0x66a7df00,
	0x6665df80, 0x6624df80, 0x65e3e000, 0x65a3e000, 0x6563e080, 0x6524e080, 0x64e5e080, 0x64a6e100,
	0x6468e180, 0x642be180, 0x63eee180, 0x63b1e200, 0x6375e200, 0x6339e200, 0x62fde280, 0x62c2e300,
	0x6288e300, 0x624ee300, 0x6214e300, 0x61dae380, 0x61a1e400, 0x6169e400, 0x6131e400, 0x60f9e400,
	0x60c1e480, 0x608ae480, 0x6053e500, 0x601de500, 0x5fe7e500, 0x5fb1e580, 0x5f7ce580, 0x5f47e580,
	0x5f12e600, 0x5edee600, 0x5eaae680, 0x5e77e600, 0x5e43e680, 0x5e10e700, 0x5ddee680, 0x5dabe700,
	0x5d79e780, 0x5d48e700, 0x5d16e780, 0x5ce5e780, 0x5cb4e800, 0x5c84e800, 0x5c54e800, 0x5c24e800,
	0x5bf4e880, 0x5bc5e880, 0x5b96e880, 0x5b67e900, 0x5b39e880, 0x5b0ae980, 0x5adde900, 0x5aafe900,
	0x5a81e980, 0x5a54e980, 0x5a27ea00, 0x59fbea00, 0x59cfe980, 0x59a2ea80, 0x5977ea00, 0x594bea80,
	0x5920ea80, 0x58f5ea80, 0x58caea80, 0x589feb00, 0x5875eb00, 0x584beb00, 0x5821eb00, 0x57f7eb80,
	0x57ceeb00, 0x57a4eb80, 0x577bec00, 0x5753eb80, 0x572aec00, 0x5702ec00, 0x56daec00, 0x56b2ec00,
	0x568aec80, 0x5663ec00, 0x563bec80, 0x5614ed00, 0x55eeec80, 0x55c7ed00, 0x55a1ec80, 0x557aed00,
	0x5554ed00, 0x552eed80, 0x5509ed00, 0x54e3ed80, 0x54beed80, 0x5499ed80, 0x5474ee00, 0x5450ed80,
	0x542bee00, 0x5407ee00, 0x53e3ee00, 0x53bfee00, 0x539bee00, 0x5377ee80, 0x5354ee80, 0x5331ee80,
	0x530eee80, 0x52ebee80, 0x52c8ef00, 0x52a6ee80, 0x5283ef00, 0x5261ef00, 0x523fef00, 0x521def00,
	0x51fbef80, 0x51daef00, 0x51b8ef80, 0x5197ef80, 0x5176ef80, 0x5155ef80, 0x5134f000, 0x5114ef80,
	0x50f3f000, 0x50d3f000, 0x50b3f000, 0x5093f000, 0x5073f000, 0x5053f080, 0x5034f000, 0x5014f080,
	0x4ff5f080, 0x4fd6f080, 0x4fb7f080, 0x4f98f080, 0x4f79f100, 0x4f5bf080, 0x4f3cf100, 0x4f1ef100,
	0x4f00f100, 0x4ee2f100, 0x4ec4f100, 0x4ea6f180, 0x4e89f100, 0x4e6bf180, 0x4e4ef100, 0x4e30f180,
	0x4e13f180, 0x4df6f180, 0x4dd9f200, 0x4dbdf180, 0x4da0f200, 0x4d84f180, 0x4d67f200, 0x4d4bf200,
	0x4d2ff200, 0x4d13f200, 0x4cf7f200, 0x4cdbf280, 0x4cc0f200, 0x4ca4f280, 0x4c89f200, 0x4c6df280,
	0x4c52f280, 0x4c37f280, 0x4c1cf280, 0x4c01f280, 0x4be6f300, 0x4bccf280, 0x4bb1f300, 0x4b97f300,
	0x4b7df280, 0x4b62f300, 0x4b48f300, 0x4b2ef300, 0x4b14f380, 0x4afbf300, 0x4ae1f300, 0x4ac7f380,
	0x4aaef300, 0x4a94f380, 0x4a7bf380, 0x4a62f380, 0x4a49f380, 0x4a30f380, 0x4a17f380, 0x49fef400,
	0x49e6f380, 0x49cdf400, 0x49b5f380, 0x499cf400, 0x4984f400, 0x496cf400, 0x4954f380, 0x493bf480,
	0x4924f400, 0x490cf400, 0x48f4f400, 0x48dcf480, 0x48c5f400, 0x48adf480, 0x4896f480, 0x487ff400,
	0x4867f480, 0x4850f480, 0x4839f480, 0x4822f480, 0x480bf500, 0x47f5f480, 0x47def480, 0x47c7f500,
	0x47b1f480, 0x479af500, 0x4784f500, 0x476ef480, 0x4757f500, 0x4741f500, 0x472bf500, 0x4715f500,
	0x46fff580, 0x46eaf500, 0x46d4f500, 0x46bef580, 0x46a9f500, 0x4693f580, 0x467ef500, 0x4668f580,
	0x4653f580, 0x463ef580, 0x4629f580, 0x4614f580, 0x45fff580, 0x45eaf580, 0x45d5f580, 0x45c0f600,
	0x45acf580, 0x4597f580, 0x4582f600, 0x456ef600, 0x455af580, 0x4545f600, 0x4531f600, 0x451df600,
	0x4509f600, 0x44f5f600, 0x44e1f600, 0x44cdf600, 0x44b9f600, 0x44a5f600, 0x4491f680, 0x447ef600,
	0x446af680, 0x4457f600, 0x4443f680, 0x4430f680, 0x441df600, 0x4409f680, 0x43f6f680, 0x43e3f680,
	0x43d0f680, 0x43bdf680, 0x43aaf680, 0x4397f680, 0x4384f700, 0x4372f680, 0x435ff680, 0x434cf700,
	0x433af680, 0x4327f700, 0x4315f680, 0x4302f700, 0x42f0f700, 0x42def680, 0x42cbf700, 0x42b9f700,
	0x42a7f700, 0x4295f700, 0x4283f700, 0x4271f700, 0x425ff700, 0x424df780, 0x423cf700, 0x422af700,
	0x4218f780, 0x4207f700, 0x41f5f780, 0x41e4f700, 0x41d2f780, 0x41c1f780, 0x41b0f700, 0x419ef780,
	0x418df780, 0x417cf780, 0x416bf780, 0x415af780, 0x4149f780, 0x4138f780, 0x4127f780, 0x4116f780,
	0x4105f780, 0x40f4f800, 0x40e4f780, 0x40d3f780, 0x40c2f800, 0x40b2f780, 0x40a1f800, 0x4091f800,
	0x4081f780, 0x4070f800, 0x4060f800, 0x4050f780, 0x403ff800, 0x402ff800, 0x401ff800, 0x400ff800,
};

const unsigned short sin_lut[(SIN_TABLE_SIZE >> 2) + 1] =
{
	0x0000, 0x0324, 0x0648, 0x096c, 0x0c8f, 0x0fb2, 0x12d5, 0x15f6, 0x1917, 0x1c37,
	0x1f56, 0x2273, 0x258f, 0x28aa, 0x2bc3, 0x2edb, 0x31f1, 0x3505, 0x3816, 0x3b26,
	0x3e33, 0x413e, 0x4447, 0x474c, 0x4a4f, 0x4d4f, 0x504d, 0x5347, 0x563e, 0x5931,
	0x5c21, 0x5f0e, 0x61f7, 0x64dc, 0x67bd, 0x6a9a, 0x6d73, 0x7048, 0x7319, 0x75e5,
	0x78ac, 0x7b6f, 0x7e2e, 0x80e7, 0x839b, 0x864a, 0x88f5, 0x8b99, 0x8e39, 0x90d3,
	0x9367, 0x95f6, 0x987f, 0x9b02, 0x9d7f, 0x9ff6, 0xa266, 0xa4d1, 0xa735, 0xa993,
	0xabea, 0xae3b, 0xb085, 0xb2c8, 0xb504, 0xb739, 0xb967, 0xbb8e, 0xbdae, 0xbfc6,
	0xc1d7, 0xc3e1, 0xc5e3, 0xc7dd, 0xc9d0, 0xcbbb, 0xcd9e, 0xcf79, 0xd14c, 0xd317,
	0xd4da, 0xd695, 0xd847, 0xd9f1, 0xdb93, 0xdd2c, 0xdebd, 0xe045, 0xe1c4, 0xe33b,
	0xe4a9, 0xe60e, 0xe76a, 0xe8be, 0xea08, 0xeb4a, 0xec82, 0xedb1, 0xeed7, 0xeff4,
	0xf108, 0xf212, 0xf313, 0xf40a, 0xf4f9, 0xf5dd, 0xf6b9, 0xf78a, 0xf852, 0xf911,
	0xf9c6, 0xfa72, 0xfb13, 0xfbab, 0xfc3a, 0xfcbe, 0xfd39, 0xfdaa, 0xfe12, 0xfe6f,
	0xfec3, 0xff0d, 0xff4d, 0xff83, 0xffb0, 0xffd2, 0xffeb, 0xfffa, 0xffff
};

const unsigned char clz_lut[256] =
{
	0x8, 0x7, 0x6, 0x6, 0x5, 0x5, 0x5, 0x5, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

const u16 /*IWRAM_DATA*/ Rand_Table[] = {
0xb7c1, 0x51a9, 0xa32b, 0xd367, 0xb617, 0x5dde, 0x66c8, 0x7a09, 0x4873, 0x2021, 
0x773b, 0xb50b, 0x761c, 0x27d0, 0x98cf, 0x4653, 0xacf1, 0x8e77, 0xd9dc, 0x9f4d, 
0xcdc7, 0x8113, 0xf183, 0xd44d, 0x528c, 0x9517, 0xc9bb, 0x83fa, 0x459a, 0x97cc, 
0x81ae, 0x1412, 0x7c50, 0xc634, 0x16a7, 0x2206, 0xcebd, 0xbf2d, 0x2603, 0x66dc, 
0xa804, 0xfeae, 0x47f0, 0x3b06, 0x233e, 0x0460, 0x2eaf, 0x6a5e, 0x8167, 0xf224, 
0xbebe, 0x5452, 0x3f51, 0x9183, 0xdac5, 0x349f, 0xd2ca, 0x18a8, 0xdd3e, 0xdfc7, 
0xd190, 0x4fc0, 0xa2d5, 0x19d3, 0x6d46, 0x731c, 0x867a, 0x6a00, 0xc467, 0x2c47, 
0x0d6a, 0x432c, 0x770d, 0x909f, 0xf9a0, 0x0ae4, 0x099d, 0xca2b, 0xb904, 0x7cce, 
0x3181, 0x1f81, 0x1490, 0x6eeb, 0x6fed, 0x4e09, 0xd9f0, 0x9bd5, 0x12fe, 0x9c7e, 
0xcd3c, 0x1836, 0xe0a7, 0x721f, 0xd1ab, 0xc206, 0xdce9, 0x2c57, 0x79ae, 0x157b, 
0x7b4b, 0x92ec, 0x7cd3, 0xf737, 0xad79, 0xedf2, 0xd725, 0x9bc3, 0x0081, 0x0a28, 
0xe6c0, 0xb963, 0x628f, 0x1643, 0xc9c8, 0xaf39, 0xe77e, 0xcc5f, 0x163a, 0xf173, 
0x0e18, 0xa7f6, 0xf362, 0x71b0, 0xab8b, 0xfd56, 0xd6a0, 0x4ece, 0x48f7, 0x8b04, 
0x9a52, 0x87ec, 0x5f40, 0x11c4, 0x814a, 0xc51f, 0xbb08, 0x801b, 0xe4fb, 0x9fbb, 
0x8268, 0x1ac0, 0xae67, 0xcb70, 0x22f3, 0xca2a, 0x7415, 0x72a3, 0xbefe, 0x9fb0, 
0x8a20, 0x15c9, 0x52d3, 0x9f2f, 0x6f12, 0x6589, 0xd99d, 0x3449, 0xe3e4, 0x0f85, 
0x5188, 0xec44, 0xa249, 0x257a, 0x83a7, 0x219c, 0x948f, 0x9a6c, 0x4b85, 0x36ef, 
0x2ccd, 0xd186, 0x41aa, 0x127b, 0xba43, 0x037f, 0x3893, 0x9562, 0x7e1f, 0x14c8, 
0x0e87, 0x4147, 0xd6d4, 0x00f7, 0x1219, 0x2c44, 0xa549, 0x5a98, 0xfadd, 0x5af9, 
0xe9a2, 0x7391, 0x0afc, 0x0a2f, 0x1323, 0x30ae, 0xe3eb, 0x3833, 0x65bd, 0x2377, 
0xb086, 0xdcde, 0x2555, 0x7f41, 0xbf6b, 0xb6c7, 0x0ef2, 0x2bb6, 0xaee5, 0x16fa, 
0xf6b7, 0x600f, 0xdb3b, 0x223e, 0x1363, 0xf594, 0x8a99, 0x8941, 0x2499, 0xd9f1, 
0x0857, 0x6aca, 0x5196, 0x3a52, 0xbe95, 0xa531, 0xdc85, 0x8bd1, 0x0a4d, 0xdc60, 
0x8fcc, 0x4abe, 0xa6d7, 0x1754, 0x0abb, 0x641f, 0xa736, 0xfe2d, 0xa1cc, 0x5ae8, 
0x8945, 0x1eea, 0x3891, 0xe505, 0x4568, 0xdd30, 0x9c2e, 0x5cc1, 0x46f2, 0x5a98, 
0x0e69, 0x780d, 0xc711, 0x0c80, 0x65c5, 0xb88a, 0x5b6f, };


