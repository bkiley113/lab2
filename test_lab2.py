# import pathlib
# import re
# import subprocess
# import unittest
# import tempfile
# import os


# class TestLab2(unittest.TestCase):
#     def _make():
#         result = subprocess.run(["make"], capture_output=True, text=True)
#         return result

#     def _make_clean():
#         result = subprocess.run(["make", "clean"], capture_output=True, text=True)
#         return result

#     @classmethod
#     def setUpClass(cls):
#         cls.make = cls._make().returncode == 0

#     @classmethod
#     def tearDownClass(cls):
#         cls._make_clean()

#     def test_averages(self):
#         fileName = "processes.txt"
#         correctAvgWaitTime = (0, 5.5, 5.0, 7, 4.5, 5.5, 6.25, 4.75)
#         correctAvgRespTime = (0, 0.75, 1.5, 2.75, 3.25, 3.25, 4, 4.75)

#         self.assertTrue(self.make, msg="make failed")
#         for x in range(1, 7):
#             cl_result = subprocess.check_output(("./rr", fileName, str(x))).decode()
#             lines = cl_result.split("\n")
#             testAvgWaitTime = float(lines[0].split(":")[1])
#             testAvgRespTime = float(lines[1].split(":")[1])

#             result = True
#             if testAvgWaitTime != correctAvgWaitTime[x]:
#                 result = False
#             if testAvgRespTime != correctAvgRespTime[x]:
#                 result = False

#             self.assertTrue(
#                 result,
#                 f"\n    Quantum Time: {x}\n Correct Results: Avg Wait. Time:{correctAvgWaitTime[x]}, Avg. Resp. Time:{correctAvgRespTime[x]}\n    Your Results: Avg Wait. Time:{testAvgWaitTime}, Avg. Resp. Time:{testAvgRespTime}\n",
#             )

#     def test_arrival_and_requeue(self):
#         self.assertTrue(self.make, msg="make failed")

#         correctAvgWaitTime = (0, 5, 5.25, 6.5, 4.0, 4.5, 5.75, 4.75)
#         correctAvgRespTime = (0, 0.75, 1.5, 2.25, 2.75, 3.25, 3.5, 4.75)

#         # temp file for this case.
#         with tempfile.NamedTemporaryFile() as f:
#             f.write(b"4\n")
#             f.write(b"1, 0, 7\n")
#             f.write(b"2, 3, 4\n")
#             f.write(b"3, 4, 1\n")
#             f.write(b"4, 6, 4\n")
#             f.flush()

#             for x in range(1, 7):
#                 cl_result = subprocess.check_output(("./rr", f.name, str(x))).decode()
#                 lines = cl_result.split("\n")
#                 testAvgWaitTime = float(lines[0].split(":")[1])
#                 testAvgRespTime = float(lines[1].split(":")[1])

#                 result = True
#                 if testAvgWaitTime != correctAvgWaitTime[x]:
#                     result = False
#                 if testAvgRespTime != correctAvgRespTime[x]:
#                     result = False

#                 self.assertTrue(
#                     result,
#                     f"\n Cannot handle re-queue and new process arrival at the same time\n   Quantum Time: {x}\n Correct Results: Avg Wait. Time:{correctAvgWaitTime[x]}, Avg. Resp. Time:{correctAvgRespTime[x]}\n    Your Results: Avg Wait. Time:{testAvgWaitTime}, Avg. Resp. Time:{testAvgRespTime}\n",
#                 )


import pathlib
import re
import subprocess
import unittest
import tempfile
import os


class TestLab2(unittest.TestCase):
    def _make():
        result = subprocess.run(["make"], capture_output=True, text=True)
        return result

    def _make_clean():
        result = subprocess.run(["make", "clean"], capture_output=True, text=True)
        return result

    @classmethod
    def setUpClass(cls):
        cls.make = cls._make().returncode == 0

    @classmethod
    def tearDownClass(cls):
        cls._make_clean()

    def test_averages(self):
        fileName = "processes.txt"
        correctAvgWaitTime = (0, 5.5, 5.0, 7, 4.5, 5.5, 6.25, 4.75)
        correctAvgRespTime = (0, 0.75, 1.5, 2.75, 3.25, 3.25, 4, 4.75)

        self.assertTrue(self.make, msg="make failed")
        for x in range(1, 7):
            cl_result = subprocess.check_output(("./rr", fileName, str(x))).decode()
            lines = cl_result.split("\n")
            testAvgWaitTime = float(lines[0].split(":")[1])
            testAvgRespTime = float(lines[1].split(":")[1])

            result = True
            if testAvgWaitTime != correctAvgWaitTime[x]:
                result = False
            if testAvgRespTime != correctAvgRespTime[x]:
                result = False

            self.assertTrue(
                result,
                f"\n    Quantum Time: {x}\n Correct Results: Avg Wait. Time:{correctAvgWaitTime[x]}, Avg. Resp. Time:{correctAvgRespTime[x]}\n    Your Results: Avg Wait. Time:{testAvgWaitTime}, Avg. Resp. Time:{testAvgRespTime}\n",
            )

    def test_arrival_and_requeue(self):
        self.assertTrue(self.make, msg="make failed")

        correctAvgWaitTime = (0, 5, 5.25, 6.5, 4.0, 4.5, 5.75, 4.75)
        correctAvgRespTime = (0, 0.75, 1.5, 2.25, 2.75, 3.25, 3.5, 4.75)

        # Temp file for this case.
        with tempfile.NamedTemporaryFile() as f:
            f.write(b"4\n")
            f.write(b"1, 0, 7\n")
            f.write(b"2, 3, 4\n")
            f.write(b"3, 4, 1\n")
            f.write(b"4, 6, 4\n")
            f.flush()

            for x in range(1, 7):
                cl_result = subprocess.check_output(("./rr", f.name, str(x))).decode()
                lines = cl_result.split("\n")
                testAvgWaitTime = float(lines[0].split(":")[1])
                testAvgRespTime = float(lines[1].split(":")[1])

                result = True
                if testAvgWaitTime != correctAvgWaitTime[x]:
                    result = False
                if testAvgRespTime != correctAvgRespTime[x]:
                    result = False

                self.assertTrue(
                    result,
                    f"\n Cannot handle re-queue and new process arrival at the same time\n   Quantum Time: {x}\n Correct Results: Avg Wait. Time:{correctAvgWaitTime[x]}, Avg. Resp. Time:{correctAvgRespTime[x]}\n    Your Results: Avg Wait. Time:{testAvgWaitTime}, Avg. Resp. Time:{testAvgRespTime}\n",
                )

    def test_single_process(self):
        """Test a single process. With only one process, waiting and response times should be zero."""
        self.assertTrue(self.make, msg="make failed")
        with tempfile.NamedTemporaryFile() as f:
            # Create a file with one process: pid=1, arrival_time=2, burst_time=10
            f.write(b"1\n")
            f.write(b"1, 2, 10\n")
            f.flush()

            # Test multiple quantum values
            for quantum in range(1, 11):
                cl_result = subprocess.check_output(("./rr", f.name, str(quantum))).decode()
                lines = cl_result.splitlines()
                avg_wait = float(lines[0].split(":")[1])
                avg_resp = float(lines[1].split(":")[1])
                # For a single process, waiting time = 0 and response time = 0.
                self.assertAlmostEqual(avg_wait, 0.0, places=2,
                                       msg=f"Quantum {quantum}: Single process avg wait should be 0, got {avg_wait}")
                self.assertAlmostEqual(avg_resp, 0.0, places=2,
                                       msg=f"Quantum {quantum}: Single process avg response should be 0, got {avg_resp}")

    def test_all_simultaneous(self):
        """Test when all processes arrive at the same time."""
        self.assertTrue(self.make, msg="make failed")
        with tempfile.NamedTemporaryFile() as f:
            f.write(b"3\n")
            # Three processes, all arriving at time 0.
            f.write(b"1, 0, 4\n")
            f.write(b"2, 0, 6\n")
            f.write(b"3, 0, 8\n")
            f.flush()

            # We'll test for quantum = 2
            quantum = 2
            cl_result = subprocess.check_output(("./rr", f.name, str(quantum))).decode()
            lines = cl_result.splitlines()
            avg_wait = float(lines[0].split(":")[1])
            avg_resp = float(lines[1].split(":")[1])
            # Expected simulation:
            # P1: waiting = 8 - 0 - 4 = 4
            # P2: waiting = 14 - 0 - 6 = 8
            # P3: waiting = 18 - 0 - 8 = 10
            # Average waiting = (4 + 8 + 10) / 3 ≈ 7.33, average response = 0.
            expected_avg_wait = 22.0 / 3.0
            expected_avg_resp = 0.0
            self.assertAlmostEqual(avg_wait, expected_avg_wait, places=2,
                                   msg="All simultaneous processes avg wait mismatch.")
            self.assertAlmostEqual(avg_resp, expected_avg_resp, places=2,
                                   msg="All simultaneous processes avg response mismatch.")

    def test_high_quantum(self):
        """Test with a very high quantum so that each process runs to completion on its turn."""
        self.assertTrue(self.make, msg="make failed")
        with tempfile.NamedTemporaryFile() as f:
            f.write(b"3\n")
            # Three processes with different arrival times.
            f.write(b"1, 0, 5\n")
            f.write(b"2, 2, 3\n")
            f.write(b"3, 4, 7\n")
            f.flush()

            # With a high quantum (e.g., 100), processes run to completion once scheduled.
            quantum = 100
            cl_result = subprocess.check_output(("./rr", f.name, str(quantum))).decode()
            lines = cl_result.splitlines()
            avg_wait = float(lines[0].split(":")[1])
            avg_resp = float(lines[1].split(":")[1])
            # Expected simulation:
            # P1: finishes at time 5, waiting = 5 - 0 - 5 = 0, response = 0.
            # P2: starts at 5, finishes at 8, waiting = 8 - 2 - 3 = 3, response = 5 - 2 = 3.
            # P3: starts at 8, finishes at 15, waiting = 15 - 4 - 7 = 4, response = 8 - 4 = 4.
            # Average waiting = (0 + 3 + 4) / 3 ≈ 2.33, average response = (0 + 3 + 4) / 3 ≈ 2.33.
            expected_avg_wait = 7.0 / 3.0
            expected_avg_resp = 7.0 / 3.0
            self.assertAlmostEqual(avg_wait, expected_avg_wait, places=2,
                                   msg="High quantum avg wait mismatch.")
            self.assertAlmostEqual(avg_resp, expected_avg_resp, places=2,
                                   msg="High quantum avg response mismatch.")

    def test_reversed_input_order(self):
        """Test with processes in reverse order to ensure sorting by arrival time works."""
        self.assertTrue(self.make, msg="make failed")
        with tempfile.NamedTemporaryFile() as f:
            # Create 3 processes in reverse order by arrival time.
            f.write(b"3\n")
            f.write(b"3, 4, 3\n")  # arrival=4, burst=3
            f.write(b"2, 2, 5\n")  # arrival=2, burst=5
            f.write(b"1, 0, 4\n")  # arrival=0, burst=4
            f.flush()

            # Test with quantum = 2
            quantum = 2
            cl_result = subprocess.check_output(("./rr", f.name, str(quantum))).decode()
            lines = cl_result.splitlines()
            avg_wait = float(lines[0].split(":")[1])
            avg_resp = float(lines[1].split(":")[1])
            # Expected simulation after sorting (by arrival time):
            # P1: arrival=0, burst=4
            # P2: arrival=2, burst=5
            # P3: arrival=4, burst=3
            # Simulated schedule (quantum=2):
            #   t=0: P1 runs 2 (rem=2), t becomes 2; queue: [P2, P1]
            #   t=2: P2 runs 2 (rem=3), t becomes 4; queue: [P1, P3, P2]
            #   t=4: P1 runs 2 (finishes), t becomes 6; waiting for P1 = 6 - 0 - 4 = 2.
            #   t=6: P3 runs 2 (rem=1), t becomes 8; queue: [P2, P3]
            #   t=8: P2 runs 2 (rem=1), t becomes 10; queue: [P3, P2]
            #   t=10: P3 runs 1 (finishes), t becomes 11; waiting for P3 = 11 - 4 - 3 = 4.
            #   t=11: P2 runs 1 (finishes), t becomes 12; waiting for P2 = 12 - 2 - 5 = 5.
            # Average waiting = (2 + 5 + 4)/3 ≈ 3.67.
            # Response times:
            #   P1: first run at 0 → response = 0.
            #   P2: first run at 2 → response = 0.
            #   P3: first run at 6 → response = 6 - 4 = 2.
            # Average response = (0 + 0 + 2)/3 ≈ 0.67.
            expected_avg_wait = 11.0 / 3.0
            expected_avg_resp = 0.67  # approximate value
            self.assertAlmostEqual(avg_wait, expected_avg_wait, places=2,
                                   msg="Reversed order avg wait mismatch.")
            self.assertAlmostEqual(avg_resp, expected_avg_resp, places=2,
                                   msg="Reversed order avg response mismatch.")


if __name__ == "__main__":
    unittest.main()
