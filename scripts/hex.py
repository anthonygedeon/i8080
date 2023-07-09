# credit gamingbuddhist

from PIL import Image
import sys

img = Image.open('8080_instruction.png')
img_height, img_width = img.size
frame = (round(img_height / 16), round(img_width / 16))

def get_image(hex: str) -> Image:
    row, col = int(hex[0], 16), int(hex[1], 16)
    return img.crop((col * frame[0], row * frame[1], (col + 1) * frame[0], (row + 1) * frame[1]))

def read_instruction(hex_str: str) -> list[Image]:
    instructions = hex_str.split(' ')
    images = [get_image(instruction) for instruction in instructions]
    return images

def create_panel(images: list) -> Image:
    panel = Image.new('RGB', (img_height, img_width))
    for i in range(len(images)):
        panel.paste(images[i], (i * frame[0], 0))
    return panel

def main(instructions: str = None):
    if instructions is None:
        instructions = input('Enter instructions: ')
    images = read_instruction(instructions)
    panel = create_panel(images)
    panel.save('8080_instruction_panel.png')

if __name__ == '__main__':
    if len(sys.argv) == 1:
        main()
    else:
        args = sys.argv[1:]
        instructions = ' '.join(args)
        main(instructions)