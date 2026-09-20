numeros = [2, 2, 2, 2, 2]
operadores = ["+", "*", "-", "/"]

for op in ["*", "/", "+", "-"]:
    i = 1
    while i <= len(operadores):
        if op == operadores[i - 1]:
            x = numeros[i - 1]
            y = numeros[i]

            if op == "*":
                numeros[i - 1] = x * y
            elif op == "/":
                numeros[i - 1] = x / y
            elif op == "+":
                numeros[i - 1] = x + y
            elif op == "-":
                numeros[i - 1] = x - y

            numeros.pop(i)
            operadores.pop(i - 1)
        else:
            i += 1

print(numeros[0])
