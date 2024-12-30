import os
from PIL import Image, ImageEnhance, ImageFilter

# CHANGE PATH TO DATASET AND BRIGHTNESS AND BLUR
input_folder = "/root/Archive/Dataset/euroc_light18_blur5"  # Folder containing input images
brightness_factor = 1.8  # Change this to adjust brightness
blur_factor = 5
sequence = 'MH_02_easy'  # None; if None -> process all sequences


def adjust_brightness_and_blur(image_path, brightness_factor, blur_factor):
    # Open the image
    image = Image.open(image_path)

    # Create a brightness enhancer object
    enhancer = ImageEnhance.Brightness(image)

    # Adjust brightness
    new_image = enhancer.enhance(brightness_factor)
    gaussImage = new_image.filter(ImageFilter.GaussianBlur(blur_factor))

    return gaussImage


def process_images(input_folder, brightness_factor, blur_factor):
    for directory in os.listdir(input_folder):
        if sequence is not None and directory != sequence: continue
        print('I am processing: ', directory)

        images_path = os.path.join(input_folder, directory, 'mav0', 'cam0', 'data')
        # Iterate over each image in the input folder
        for filename in os.listdir(images_path):
            if filename.endswith(".jpg") or filename.endswith(".png"):
                # Get the full path of the image
                image_path = os.path.join(images_path, filename)

                # Adjust brightness and blur
                modified_image = adjust_brightness_and_blur(image_path, brightness_factor, blur_factor)

                # Save the modified image with the same name in the output folder
                modified_image.save(image_path)


process_images(input_folder, brightness_factor, blur_factor)
filter = ImageEnhance.Contrast(img)