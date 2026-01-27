import serial
import pygame
import colorsys


SERIAL_PORT = 'COM11' #ustawić odpowiedni port esp
BAUD_RATE = 115200


X_MIN, X_MAX = 45, 135
Y_MIN, Y_MAX = 60, 120

SCALE = 8
WINDOW_WIDTH = (X_MAX - X_MIN + 1) * SCALE
WINDOW_HEIGHT = (Y_MAX - Y_MIN + 1) * SCALE

MAX_DIST_MM = 1000.0

pygame.init()
screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
pygame.display.set_caption("LIDAR Real-time: Heatmap")

canvas = pygame.Surface((WINDOW_WIDTH, WINDOW_HEIGHT))
canvas.fill((0, 0, 50))


def get_color(distance):
    if distance <= 0:
        return (10, 10, 40)


    MAX_CONTRAST_DIST = 1000.0

    d = min(distance, MAX_CONTRAST_DIST)


    norm = (d / MAX_CONTRAST_DIST) ** 0.7


    colors = [
        (255, 255, 200),
        (255, 60, 0),
        (150, 0, 100),
        (30, 0, 150),
        (10, 10, 40)
    ]

    num_colors = len(colors)
    idx = norm * (num_colors - 1)
    i1 = int(idx)
    i2 = min(i1 + 1, num_colors - 1)
    f = idx - i1

    r = int(colors[i1][0] + (colors[i2][0] - colors[i1][0]) * f)
    g = int(colors[i1][1] + (colors[i2][1] - colors[i1][1]) * f)
    b = int(colors[i1][2] + (colors[i2][2] - colors[i1][2]) * f)

    return (r, g, b)


try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
    ser.flushInput()
    print(f"Połączono z {SERIAL_PORT}")
except Exception as e:
    print(f"Błąd portu: {e}")
    exit()

clock = pygame.time.Clock()
running = True

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_c:
                canvas.fill(get_color(MAX_DIST_MM))

    lines_processed = 0
    while ser.in_waiting > 0:
        try:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            parts = line.split(',')

            if len(parts) == 3:
                dist = int(parts[0])
                pos_x = int(parts[1])
                pos_y = int(parts[2])

                if (X_MIN <= pos_x <= X_MAX) and (Y_MIN <= pos_y <= Y_MAX):
                    draw_x = (pos_x - X_MIN) * SCALE
                    draw_y = (Y_MAX - pos_y) * SCALE

                    color = get_color(dist)

                    pygame.draw.rect(canvas, color, (draw_x, draw_y, SCALE, SCALE))
                    lines_processed += 1
        except ValueError:
            pass
        except Exception:
            pass

    if lines_processed > 0:
        screen.blit(canvas, (0, 0))
        pygame.display.flip()

    clock.tick(60)

ser.close()
pygame.quit()
