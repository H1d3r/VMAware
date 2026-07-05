## Security Policy and Reporting Guidelines

Thank you for your interest in contributing to the security of **VMAware**, a C++ library for VM detection. 
We take security vulnerabilities seriously and aim to fix them promptly and responsibly. Please read and follow these guidelines when reporting a security issue.

---

### 1. Supported Versions

We only provide security fixes for the latest major release versions. 
If you discover a vulnerability in an unsupported version, please upgrade to the latest version in our Releases and verify whether the issue still exists.

---

### 2. Reporting a Vulnerability

**Do not** open a public issue for security vulnerabilities.

**Contact**: Submit your report via email to the maintainers at:

```
vmaware.support@gmail.com
```

**Information to Include**:

* A clear description of the vulnerability.
* Steps to reproduce (proof-of-concept if available).
* Impact assessment (what threats this poses).
* Affected versions (as per Section 1).
* Suggested remediation or patches if possible.

---

### 3. Triage Policy

We prioritize meaningful and actionable security vulnerabilities. Reports will be dismissed without disclosure or fix if they:

> Have no real-world impact.

> Require unrealistic conditions for exploitation.

> Relate to outdated or unsupported versions.

We reserve the right to classify any report as non-actionable.

### 4. Handling and Response Timeline

**__1.__** Acknowledgment: You will receive a response within one week.

**__2.__** Evaluation: Investigation will begin within 5 business days.

**__3.__** Patch Development: If the issue is valid, a fix will be developed within 30 calendar days.

**__4.__** Disclosure: For actionable vulnerabilities, we will issue a security advisory and optionally credit the reporter.

### 5. Post-Fix Actions

* The patched version will be tagged with a security banner (e.g., v2.9.0-secfix).
* We will coordinate with package repositories and downstream projects.