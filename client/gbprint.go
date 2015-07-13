package main

import (
	"flag"
	"image"
	"image/color"
	_ "image/gif"
	"image/jpeg"
	_ "image/png"
	"log"
	"os"
	"time"

	"github.com/nfnt/resize"
	"github.com/tarm/serial"
)

// Declare GB Printer color palette
// Game Boy Printer has a 2bit greyscale color depth
// Only 4 colors: White, light grey, dark grey and black, in that order
var gbpPalette = color.Palette{
	color.Gray{255}, // 0x00 White
	color.Gray{170}, // 0x01 Light gray
	color.Gray{85},  // 0x02 Dark gray
	color.Gray{0},   // 0x03 Black
}

// Dithering algorithms
func ditheringFloydSteinberg(img *image.Gray, p *color.Palette) {
	size := img.Bounds()
	for y := size.Min.Y; y < size.Max.Y; y++ {
		for x := size.Min.X; x < size.Max.X; x++ {
			c1 := img.GrayAt(x, y)
			c2 := p.Convert(c1).(color.Gray)
			delta := int(c1.Y) - int(c2.Y)
			switch {
			case x+1 < size.Max.X && y < size.Max.Y:
				img.SetGray(x+1, y, color.Gray{uint8(int(img.GrayAt(x+1, y).Y) + delta*7/16)})
				fallthrough
			case x < size.Max.X && y+1 < size.Max.Y:
				img.SetGray(x, y+1, color.Gray{uint8(int(img.GrayAt(x, y+1).Y) + delta*5/16)})
				fallthrough
			case x-1 >= size.Min.X && y+1 < size.Max.Y:
				img.SetGray(x-1, y+1, color.Gray{uint8(int(img.GrayAt(x-1, y+1).Y) + delta*3/16)})
				fallthrough
			case x+1 < size.Max.X && y+1 < size.Max.Y:
				img.SetGray(x+1, y+1, color.Gray{uint8(int(img.GrayAt(x+1, y+1).Y) + delta*1/16)})
			}
			img.Set(x, y, c2)
		}
	}
}

// Transforms gray input image color depth to the palette defined in p
// The algorithm simply assigns the nearest palette color to each pixel,
// without distributing error in any way, so expect color banding
func ditheringNone(img *image.Gray, p *color.Palette) {
	size := img.Bounds()
	for y := size.Min.Y; y < size.Max.Y; y++ {
		for x := size.Min.X; x < size.Max.X; x++ {
			c1 := img.GrayAt(x, y)
			c2 := p.Convert(c1).(color.Gray)
			img.SetGray(x, y, c2)
		}
	}
}

func openImg(path string) (img image.Image, err error) {
	// So far this openImg function supports as many formats as the import statements on this file
	// declare. As of writing this comment this is:
	// - gif
	// - jpeg
	// - png
	r, err := os.Open(path)
	defer r.Close()
	if err != nil {
		return nil, err
	}
	srcImg, _, err := image.Decode(r)

	return srcImg, err
}

func saveJpg(path string, img image.Image) error {
	w, err := os.Create(path)
	if err != nil {
		return err
	}
	defer w.Close()
	err = jpeg.Encode(w, img, nil)
	return err
}

// The GBPrinter image format requires the image to be sent tile by tile, instead of line by line.
// Each tile is made of a 8x8 pixels and must be serialized top down, left right. After each
// pixel in a tile has been added to the channel, the loop goes onto the next tile, until the pixels
// have been fully fed to the channel
func readPixelsByTiles(img image.Image) <-chan byte {
	pixels := make(chan byte)
	go func() {
		for boxY := 0; boxY < img.Bounds().Max.Y; boxY += 8 {
			for boxX := 0; boxX < img.Bounds().Max.X; boxX += 8 {
				for y := boxY; y < boxY+8; y++ {
					for x := boxX; x < boxX+8; x++ {
						pixels <- byte(gbpPalette.Index(img.At(x, y)))
					}
				}
			}
		}
		close(pixels)
	}()
	return pixels
}

// Write pixels read from chan pixels to buffer. Each pixel will have values of [0, 3], so it fits
// in 2 bits. Each byte holds 4 pixels thus.
func writePixelsToBuffer(pixels <-chan byte, buffer []byte) {
	for idx := range buffer {
		for i := 6; i >= 0; i -= 2 {
			buffer[idx] |= <-pixels << uint(i)
		}
	}
}

// Declare variables to store CLI flag values
var fSerial, fDither, fSave string
var fNoRotate, fNoResize bool

func init() {
	flag.StringVar(&fSerial, "serial", "", "Serial interface to connect to gbprinter [ex: /dev/tty.usb1]")
	flag.StringVar(&fDither, "dither", "FLOYDSTEINBERG", "Dithering algorithm to apply. "+
		"Options: [NONE|FLOYDSTEINBERG(DEFAULT)]")
	flag.StringVar(&fSave, "save", "", "Save path for resulting image (jpg format). "+
		"Useful to preview the printing results. Only applies if -serial flag disabled")

	flag.BoolVar(&fNoRotate, "no-rotate", false, "Don't rotate the resulting image. "+
		"Only applies if -save flag enabled")
	flag.BoolVar(&fNoResize, "no-resize", false, "Don't resize the resulting image. "+
		"Only applies if -save flag enabled")
}

func main() {
	// Parse CLI flags and deal with basic input errors
	flag.Parse()

	if flag.Arg(0) == "" {
		log.Fatalln("You must specify an image src path")
	}

	if fSerial == "" && fSave == "" {
		log.Fatalln("You must specify either -serial or -save flags")
	}

	switch fDither {
	case "NONE":
	case "FLOYDSTEINBERG":
	default:
		log.Fatalln("Specify a dithering algorithm from the list shown in -help")
	}

	// Read img source
	srcImg, err := openImg(flag.Arg(0))
	if err != nil {
		log.Fatalln(err.Error())
	}

	var saveImage, resizeRequired, rotationRequired bool
	// Only save image if !-serial and -save is active
	saveImage = fSerial == "" && fSave != ""
	// Detect if rotation is needed (we want img Max.Y >= Max.X) to print via serial
	// If  !-serial and -save and -no-rotate are active, do not rotate
	rotationRequired = srcImg.Bounds().Max.Y < srcImg.Bounds().Max.X && !(saveImage && fNoRotate)
	// Resize won't be required only in the case that we want to save the image and fNoResize has
	// been set
	resizeRequired = !(saveImage && fNoResize)

	// Resize image, if required
	if resizeRequired {
		if rotationRequired {
			srcImg = resize.Resize(0, 160, srcImg, resize.Lanczos3)

		} else {
			srcImg = resize.Resize(160, 0, srcImg, resize.Lanczos3)
		}
	}

	size := srcImg.Bounds()
	var grayImg *image.Gray

	// Convert to grayscale & rotate (if required)
	if rotationRequired {
		grayImg = image.NewGray(image.Rect(0, 0, size.Max.Y, size.Max.X))
		// Convert to Gray colorspace
		for y := size.Min.Y; y < size.Max.Y; y++ {
			for x := size.Min.X; x < size.Max.X; x++ {
				grayImg.Set(size.Max.Y-1-y, x, srcImg.At(x, y))
			}
		}
	} else {
		grayImg = image.NewGray(image.Rect(0, 0, size.Max.X, size.Max.Y))
		// Convert to Gray colorspace
		for y := size.Min.Y; y < size.Max.Y; y++ {
			for x := size.Min.X; x < size.Max.X; x++ {
				grayImg.Set(x, y, srcImg.At(x, y))
			}
		}
	}

	// Apply selected dithering algorithm
	switch fDither {
	case "NONE":
		ditheringNone(grayImg, &gbpPalette)
	case "FLOYDSTEINBERG":
		ditheringFloydSteinberg(grayImg, &gbpPalette)
	}

	// Save image (only if !-serial and -save)
	if saveImage {
		err = saveJpg(fSave, grayImg)
		if err != nil {
			log.Fatalln(err.Error())
		}
		log.Printf("Transformation done. Image saved at %s\n", fSave)
		return // Exit here. Don't start serial communication if saving file to an external file
	}

	// Serialize transformed img into a byte array we can send to the GBPrinter
	imgBuffer := make([]byte, grayImg.Bounds().Max.X*grayImg.Bounds().Max.Y/4)
	writePixelsToBuffer(readPixelsByTiles(grayImg), imgBuffer)

	c := &serial.Config{Name: fSerial, Baud: 9600, ReadTimeout: time.Second * 500}
	s, err := serial.OpenPort(c)
	if err != nil {
		log.Fatalln(err.Error())
	}

	msgBuf4 := make([]byte, 4)
	msgBuf2 := make([]byte, 2)
	for idx := range msgBuf4 {
		msgBuf4[idx] = byte(uint32(len(imgBuffer)) & (uint32(0xFF) << uint((3-idx)*8)) >> uint((3-idx)*8))
	}
	log.Println(msgBuf4)
	_, err = s.Write(msgBuf4)
	if err != nil {
		log.Fatal(err)
	}
	_, err = s.Read(msgBuf4)
	log.Println(msgBuf4)
	if err != nil {
		log.Fatal(err)
	}
	s.Write([]byte("OK"))
	_, err = s.Read(msgBuf2)
	log.Println(msgBuf2)
	if err != nil {
		log.Fatal(err)
	}
	buf_size := 1280
	for idx := 0; idx < len(imgBuffer); idx += buf_size {
		var remaining int
		if idx+buf_size > len(imgBuffer) {
			remaining = len(imgBuffer)
		} else {
			remaining = idx + buf_size
		}
		s.Write(imgBuffer[idx:remaining])
		s.Read(msgBuf4)
		log.Println(msgBuf4)
	}
	log.Println("DONE!")
}
