import numpy as np

# Функция для подсчета значений в массиве
def replace_with_counts(arr):
    # Применяем np.bincount для подсчета уникальных значений
    counts = np.bincount(arr)
    # Заменяем каждое значение в исходном массиве на его количество
    return counts[arr]

if __name__ == "__main__":
    # Тест 1 массив с несколькими уникальными значениями
    arr1 = np.array([1, 4, 4, 2, 3, 3])
    result1 = replace_with_counts(arr1)
    print("Тест 1:", result1)  # Ожидается: [1 2 2 1 2 2]

    # Тест 2 массив с нулями
    arr2 = np.array([0, 0, 1, 1, 1, 2])
    result2 = replace_with_counts(arr2)
    print("Тест 2:", result2)  # Ожидается: [2 2 3 3 3 1]

    # Тест 3 массив с большим количеством повторяющихся элементов и уникальными значениями.
    arr3 = np.array([5, 5, 5, 1, 1])
    result3 = replace_with_counts(arr3)
    print("Тест 3:", result3)  # Ожидается: [3 3 3 2 2]

    # Тест 4 массив с единственным элементом
    arr4 = np.array([1])
    result4 = replace_with_counts(arr4)
    print("Тест 4:", result4)  # Ожидается: [1]

    # Тест 5 массив с одинаковыми элементами
    arr5 = np.array([2, 2, 2, 2])
    result5 = replace_with_counts(arr5)
    print("Тест 5:", result5)  # Ожидается: [4 4 4 4]