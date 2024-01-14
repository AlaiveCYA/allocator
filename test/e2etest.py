import subprocess
import unittest

class TestMyAllocator(unittest.TestCase):
    def test_allocator_correct(self):
        # Uruchomienie programu i przechwycenie stdout i stderr
        result = subprocess.run(['./e2etest_correct'], capture_output=True, text=True)
        stdout = result.stdout
        stderr = result.stderr
        stats = self.parse_allocator_stats(stderr)

        # Sprawdzenie, czy program zakończył się poprawnie
        self.assertEqual(result.returncode, 0, "Program zakończył się poprawnie")

        # Sprawdzenie statystyk alokatora
        self.assertEqual(stats['allocations'], 1, "Allocations should be equal to 1")
        self.assertEqual(stats['sbrk_calls'], 1, "Sbrk calls should be equal to 1")
        self.assertEqual(stats['not_freed_blocks'], 0, "Not freed blocks should be equal to 0")
        self.assertEqual(stats['bytes_allocated'], 16, "Bytes allocated should be equal to 80")
        self.assertEqual(stats['avg_bytes_allocated'], 16, "Avg bytes allocated should be equal to 80")
        self.assertEqual(stats['mem_peak'], 16, "Mem peak should be equal to 80")

    def test_allocator_faulty(self):
        # Uruchomienie programu i przechwycenie stdout i stderr
        result = subprocess.run(['./e2etest_faulty'], capture_output=True, text=True)

        # Sprawdzenie, czy program zakończył się poprawnie
        self.assertLess(result.returncode, 0, "Program zakończył się poprawnie")

    def parse_allocator_stats(self, output):

        # Przetwarzanie wyjścia programu i zwracanie statystyk alokatora jako słownika
        stats = {}
        for line in output.split('\n'):
            if line.startswith('Total number of allocations:'):
                stats['allocations'] = int(line.split(':')[1].strip())
            elif line.startswith('Total number of sbrk calls:'):
                stats['sbrk_calls'] = int(line.split(':')[1].strip())
            elif line.startswith('Total number of not freed blocks:'):
                stats['not_freed_blocks'] = int(line.split(':')[1].strip())
            elif line.startswith('Total number of bytes allocated:'):
                stats['bytes_allocated'] = int(line.split(':')[1].strip())
            elif line.startswith('Average number of bytes allocated:'):
                stats['avg_bytes_allocated'] = int(line.split(':')[1].strip())
            elif line.startswith('Peak memory usage:'):
                stats['mem_peak'] = int(line.split(':')[1].strip())
        return stats

if __name__ == '__main__':
    unittest.main()